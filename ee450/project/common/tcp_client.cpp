#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <future>
#include <mutex>
#include <utility>
#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tcp_client.h"
#include "config.h"
#include "logger.h"
#include "utils.h"

TCPClient::TCPClient() {}

TCPClient::~TCPClient() {
    this->stop();
    while (wait(nullptr) > 0) ;
    for (auto stack : this->m_stacks) {
        delete[] stack;
    }
    this->m_stacks.clear();
}

TCPClient &TCPClient::operator=(const TCPClient &) {
    return instance();
}

TCPClient::TCPClient(const TCPClient &) {}

TCPClient &TCPClient::instance() {
    static TCPClient instance;
    return instance;
}

void TCPClient::start(const uint n_workers) {
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_setrobust(&att, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&this->m_requests_mtx, &att);
    pthread_mutex_init(&this->m_results_mtx, &att);

    this->m_try_stop = false;
    this->m_stacks.clear();
    for (auto i = 0u; i < n_workers; ++i) {
        auto stack = new uint8_t[WORKER_STACK_SIZE];
        auto result = ::clone([] (void *arg) { return ((TCPClient *) arg)->worker_process(); },
                stack + WORKER_STACK_SIZE, CLONE_VM | CLONE_FILES | SIGCHLD, this);
        if (result < 0) {
            throw std::runtime_error("Clone process error");
        }
        this->m_stacks.push_back(stack);
    }
}

void TCPClient::stop() {
    this->m_try_stop = true;
}

std::future<std::vector<std::vector<uint8_t>>> TCPClient::request(const in_addr_t iaddr, const in_port_t port,
                                                                  const std::vector<std::vector<uint8_t>> &msg,
                                                                  const std::string client_name,
                                                                  const std::string on_connect_msg,
                                                                  const std::vector<std::string> &on_send_msg,
                                                                  const std::string on_finish_msg,
                                                                  const size_t buffer_size) {
    int socket_fd;
    if ((socket_fd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Socket created failed");
    }
    Utils::set_tcp_nodelay(socket_fd);

    auto request = new TCPClient::__request();
    request->fd = socket_fd;
    request->iaddr = iaddr;
    request->port = port;
    request->msg_bytes = msg;
    request->client_name = client_name;
    request->on_connect_msg = on_connect_msg;
    request->on_send_msg = on_send_msg;
    request->on_finish_msg = on_finish_msg;
    request->buffer_size = buffer_size;

    {
        pthread_mutex_lock(&this->m_results_mtx);
        this->m_result.insert(std::make_pair(socket_fd, std::promise<std::vector<std::vector<uint8_t>>>()));
        pthread_mutex_unlock(&this->m_results_mtx);
    }
    auto &promise = this->m_result.at(socket_fd);
    {
        pthread_mutex_lock(&this->m_requests_mtx);
        this->m_requests.push(request);
        pthread_mutex_unlock(&this->m_requests_mtx);
    }
    return std::move(promise.get_future());
}

int TCPClient::worker_process() {
    for (;;) {
        if (this->m_try_stop) {
            break;
        }
        if (this->m_requests.empty()) {
            ::nanosleep((const timespec[]){{0, 1000L}}, nullptr);
            continue;
        }
        TCPClient::__request *request = nullptr;
        {
            pthread_mutex_lock(&this->m_requests_mtx);
            if (this->m_requests.empty()) {
                pthread_mutex_unlock(&this->m_requests_mtx);
                continue;
            }
            request = this->m_requests.front();
            this->m_requests.pop();
            pthread_mutex_unlock(&this->m_requests_mtx);
        }
        if (request == nullptr) {
            continue;
        }
        auto result = std::vector<std::vector<uint8_t>>();
        const int socket_fd = request->fd;
        sockaddr_in serv_addr, addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = request->iaddr;
        serv_addr.sin_port = request->port;
        socklen_t len = sizeof(serv_addr);
        if (::connect(socket_fd, (sockaddr *) &serv_addr, len) < 0) {
            Logger::error("Socket " + std::to_string(socket_fd) + " connection failed");
            this->m_result.at(socket_fd).set_value(result);
            ::close(socket_fd);
            continue;
        }
        if (::getsockname(socket_fd, (sockaddr *) &addr, &len) >= 0) {
            Logger::info(request->client_name + " has TCP port " + std::to_string(ntohs(addr.sin_port)) +
                    " and IP address " + Utils::to_ip_addr(addr.sin_addr.s_addr));
        }
        if (!request->on_connect_msg.empty()) {
            Logger::info(request->on_connect_msg);
        }

        bool error = false;
        uint i = 0;
        for (auto &line : request->msg_bytes) {
            auto msg = new uint8_t[line.size() + 1];
            std::memcpy(msg, line.data(), line.size());
            msg[line.size()] = MSG_DELIMITER;

            if (::send(socket_fd, msg, line.size() + 1, 0) < 0) {
                Logger::error("Socket " + std::to_string(socket_fd) + " send failed");
                this->m_result.at(socket_fd).set_value(result);
                ::close(socket_fd);
                error = true;
                break;
            }
            // delete[] msg;
            if (i < request->on_send_msg.size()) {
                Logger::info(request->on_send_msg.at(i));
            }
            ++i;
        }
        if (error) {
            continue;
        }
        auto eof = new uint8_t[2]{MSG_DELIMITER};
        if (::send(socket_fd, eof, 1, 0) < 0) {
            Logger::error("Socket " + std::to_string(socket_fd) + " send failed");
            this->m_result.at(socket_fd).set_value(result);
            ::close(socket_fd);
            continue;
        }
        // delete[] eof;

        auto buffer = new uint8_t[request->buffer_size];
        auto recv_size = 0L;
        while ((recv_size = ::recv(socket_fd, buffer, request->buffer_size, 0)) > 0) {
            result.push_back(std::vector<uint8_t>(buffer, buffer + recv_size));
        }
        Logger::info(request->on_finish_msg);

        ::shutdown(socket_fd, SHUT_RDWR);
        {
            pthread_mutex_lock(&this->m_results_mtx);
            this->m_result.at(socket_fd).set_value(result);
            this->m_result.erase(socket_fd);
            pthread_mutex_unlock(&this->m_results_mtx);
        }
        while (::recv(socket_fd, buffer, request->buffer_size, 0) > 0) ;
        ::close(socket_fd);
        // delete request;
        // delete[] buffer;
    }
    return EXIT_SUCCESS;
}
