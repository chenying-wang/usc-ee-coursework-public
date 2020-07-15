#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tcp_server.h"
#include "logger.h"
#include "utils.h"

TCPServer::TCPServer(const in_addr_t iaddr, const in_port_t port,
                     const uint buffer_size, const uint max_queued_requests,
                     const uint max_events) :
    m_buffer_size(buffer_size),
    m_max_queued_requests(max_queued_requests),
    m_max_events(max_events) {
    if ((this->m_socket_fd = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw std::runtime_error("Socket created failed");
    }
    const int opt = 1;
    if (::setsockopt(this->m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Set socket option failed");
    }
    Utils::set_socket_reuseaddr(this->m_socket_fd);

    this->m_started = false;
    this->m_try_stop = false;
    this->m_addr.sin_family = AF_INET;
    this->m_addr.sin_addr.s_addr = iaddr;
    this->m_addr.sin_port = port;
}

TCPServer::~TCPServer() {
    if (this->m_started) {
        this->stop();
    }
    while (wait(nullptr) > 0) ;
    for (auto stack : this->m_stacks) {
        delete[] stack;
    }
    this->m_stacks.clear();
}

void TCPServer::start(const uint n_worker_processes) {
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_setrobust(&att, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&this->m_ctx_mtx, &att);
    pthread_mutex_init(&this->m_fds_mtx, &att);

    if (this->m_started) {
        return;
    }

    this->m_epoll_fd  = ::epoll_create1(0);
    if (this->m_epoll_fd < 0) {
        throw std::runtime_error("Epoll instance created failed");
    }
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = this->m_socket_fd;
    if (::epoll_ctl(this->m_epoll_fd, EPOLL_CTL_ADD, this->m_socket_fd, &ev) < 0) {
        throw std::runtime_error("Add socket into epoll instance failed");
    }

    if (::bind(this->m_socket_fd, (sockaddr *) &this->m_addr, sizeof(sockaddr_in)) < 0) {
        throw std::runtime_error("Socket " + std::to_string(this->m_socket_fd) + " bind failed");
    }
    Utils::set_socket_nonblocking(this->m_socket_fd);
    if (::listen(this->m_socket_fd, this->m_max_queued_requests) < 0) {
        throw std::runtime_error("Socket " + std::to_string(this->m_socket_fd) + " listen failed");
    }

    this->m_started = true;

    this->m_stacks.clear();
    for (auto i = 0u; i < n_worker_processes; ++i) {
        auto stack = new uint8_t[WORKER_STACK_SIZE];
        auto result = ::clone([] (void *arg) { return ((TCPServer *) arg)->worker_process(); },
                stack + WORKER_STACK_SIZE, CLONE_VM | CLONE_FILES | SIGCHLD, this);
        if (result < 0) {
            throw std::runtime_error("Clone process error");
        }
        this->m_stacks.push_back(stack);
    }
    this->master_process();
}

void TCPServer::stop() {
    if (!this->m_started) {
        return;
    }
    ::shutdown(this->m_socket_fd, SHUT_RDWR);
    ::close(this->m_socket_fd);
    this->m_socket_fd = 0;
    this->m_started = false;
}

std::vector<std::vector<uint8_t>> TCPServer::handle(const std::vector<std::vector<uint8_t>> &) {
    throw std::runtime_error("Not supported");
}

void TCPServer::post_send() {}

std::pair<in_addr_t, in_port_t> TCPServer::get_server_info() const {
    return std::make_pair(this->m_addr.sin_addr.s_addr, this->m_addr.sin_port);
}

int TCPServer::master_process() {
    epoll_event ev;
    auto events = new epoll_event[this->m_max_events];

    sockaddr_in client_addr;
    socklen_t socket_len = sizeof(sockaddr_in);
    auto buffer = new uint8_t[this->m_buffer_size];
    auto count = 0;
    for (;;) {
        if (this->m_try_stop && this->m_accept_fds.empty() && this->m_finish_fds.empty()) {
            this->stop();
            return EXIT_SUCCESS;
        }

        const auto n_fds = ::epoll_wait(this->m_epoll_fd, events, this->m_max_events, 500);
        for (auto i = 0; i < n_fds; ++i) {
            const auto fd = events[i].data.fd;
            if ((events[i].events & EPOLLERR) || (!(events[i].events & EPOLLIN))) {
                ::close(fd);
                continue;
            } else if (fd == this->m_socket_fd) {
                auto new_socket_fd = ::accept(this->m_socket_fd, (sockaddr *) &client_addr, &socket_len);
                if (new_socket_fd < 0) {
                    Logger::error("Accept failed");
                    continue;
                }
                try {
                    Utils::set_socket_nonblocking(new_socket_fd);
                } catch(const std::exception& e) {
                    Logger::error(e.what());
                    continue;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_socket_fd;
                if (::epoll_ctl(this->m_epoll_fd, EPOLL_CTL_ADD, new_socket_fd, &ev) < 0) {
                    Logger::error("Add incoming socket " + std::to_string(new_socket_fd) +
                            " into epoll instance failed");
                    continue;
                }
                {
                    pthread_mutex_lock(&this->m_ctx_mtx);
                    this->m_ctx.insert(std::make_pair(new_socket_fd, std::vector<std::vector<uint8_t>>()));
                    pthread_mutex_unlock(&this->m_ctx_mtx);
                }
                continue;
            }
            {
                pthread_mutex_lock(&this->m_fds_mtx);
                if (!this->m_wait_fds.count(fd)) {
                    this->m_accept_fds.push(fd);
                    this->m_wait_fds.insert(fd);
                }
                pthread_mutex_unlock(&this->m_fds_mtx);
            }
        }

        if (!this->m_finish_fds.empty()) {
            pthread_mutex_lock(&this->m_fds_mtx);
            while (!this->m_finish_fds.empty()) {
                auto fd = this->m_finish_fds.front();
                this->m_finish_fds.pop();
                if (::shutdown(fd, SHUT_RDWR) < 0) {
                    Logger::error("Shutdown socket " + std::to_string(fd) + " failed");
                }
                {
                    pthread_mutex_lock(&this->m_ctx_mtx);
                    this->m_ctx.erase(fd);
                    pthread_mutex_unlock(&this->m_ctx_mtx);
                }
                while (::recv(fd, buffer, this->m_buffer_size, 0) > 0) ;
                if (::close(fd) < 0) {
                    Logger::error("Close socket " + std::to_string(fd) + " failed");
                }
            }
            pthread_mutex_unlock(&this->m_fds_mtx);
        }
        ++ count;
    }
    delete[] buffer;
    delete[] events;
    return EXIT_SUCCESS;
}

int TCPServer::worker_process() {
    for (;;) {
        if (this->m_try_stop && this->m_accept_fds.empty()) {
            return EXIT_SUCCESS;
        }

        if (this->m_accept_fds.empty()) {
            ::nanosleep((const timespec[]){{0, 1000L}}, nullptr);
            continue;
        }
        auto fd = -1;
        {
            pthread_mutex_lock(&this->m_fds_mtx);
            if (this->m_accept_fds.empty()) {
                pthread_mutex_unlock(&this->m_fds_mtx);
                continue;
            }
            fd = this->m_accept_fds.front();
            this->m_accept_fds.pop();
            pthread_mutex_unlock(&this->m_fds_mtx);
        }
        if (fd < 1) {
            continue;
        }
        const auto result = this->process(fd);
        pthread_mutex_lock(&this->m_fds_mtx);
        this->m_wait_fds.erase(fd);
        if (!result) {
            this->m_finish_fds.push(fd);
        }
        pthread_mutex_unlock(&this->m_fds_mtx);
    }
    return EXIT_SUCCESS;
}

int TCPServer::process(const int socket_fd) {
    Logger::debug("Socket " + std::to_string(socket_fd) + " start processing");
    auto buffer = new uint8_t[this->m_buffer_size] ;
    auto recv_size = 0;
    while ((recv_size = ::recv(socket_fd, buffer, this->m_buffer_size, 0)) > 0) {
        auto i = buffer, j = buffer;
        auto eof = false;

        while ((i = (uint8_t *) std::memchr(j, MSG_DELIMITER, buffer + recv_size - j)) != nullptr) {
            if (i == j) {
                eof = true;
                break;
            }
            {
                pthread_mutex_lock(&this->m_ctx_mtx);
                this->m_ctx.at(socket_fd).push_back(std::vector<uint8_t>(j, i));
                pthread_mutex_unlock(&this->m_ctx_mtx);
            }
            j = i + 1;
        }
        if (!eof) {
            ::nanosleep((const timespec[]){{0, 1000L}}, nullptr);
            continue;
        }
        Logger::debug("Socket " + std::to_string(socket_fd) + " finish processing");

        try {
            auto response = this->handle(this->m_ctx.at(socket_fd));
            auto [msg_bytes, msg_bytes_size] = Utils::serialize(response);
            if (::send(socket_fd, msg_bytes, msg_bytes_size, 0) < 0) {
                // delete[] msg_bytes;
                throw std::runtime_error("Send failed");
            }
            // delete[] msg_bytes;
        } catch (const std::exception &e) {
            Logger::error("Handle TCP connection (fd: " + std::to_string(socket_fd) + ") error " + e.what());
        }
        this->post_send();
        {
            pthread_mutex_lock(&this->m_ctx_mtx);
            for (auto &line : this->m_ctx.at(socket_fd)) {
                line.clear();
            }
            this->m_ctx.at(socket_fd).clear();
            pthread_mutex_unlock(&this->m_ctx_mtx);
        }
        return 0;
    }
    Logger::debug("Socket " + std::to_string(socket_fd) + " continue processing");
    delete[] buffer;
    return 1;
}
