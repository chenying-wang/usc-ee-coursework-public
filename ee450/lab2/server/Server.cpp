#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ADDR INADDR_ANY
#define PORT (4034)

#define MAX_QUEUED_REQUESTS (0x1ff)
#define MAX_EVENTS (0x1ff)
#define BUFFER_SIZE (0xff)

#ifndef __EE450_TCP_SERVER
#define __EE450_TCP_SERVER

class TCPServer {
private:
    bool m_started;
    int m_socket_fd;
    sockaddr_in m_addr;
    void process(const int socket_fd) throw ();
    int set_nonblocking(const int fd) const throw();

protected:
    std::string to_ip_addr(const uint32_t iaddr) const;
    virtual std::vector<uint8_t> handle(const std::vector<uint8_t> &msg) throw ();

public:
    TCPServer(const in_addr_t addr, const in_port_t port) throw ();
    virtual ~TCPServer();

    void start() throw ();
    void stop() throw ();
};

#endif

#ifndef __EE450_DHCP_SERVER
#define __EE450_DHCP_SERVER

class DHCPServer : public TCPServer {
private:
    std::mt19937 mt;
    std::uniform_int_distribution<uint32_t> dist;

protected:
    std::vector<uint8_t> handle(const std::vector<uint8_t> &msg) throw () override;

public:
    DHCPServer(const in_addr_t addr, const in_port_t port) throw ();
    virtual ~DHCPServer();
};

#endif

#ifndef __EE450_SIMPLE_DHCP_MESSAGE
#define __EE450_SIMPLE_DHCP_MESSAGE

class SimpleDHCPMessage {
public:
    uint32_t xid;
    uint32_t iaddr;
};

#endif

TCPServer::TCPServer(const in_addr_t addr, const in_port_t port) throw () {
    if ((this->m_socket_fd = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw std::runtime_error("Socket created failed");
    }
    const int opt = 1;
    if (::setsockopt(this->m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Set socket option failed");
    }

    this->m_started = false;
    this->m_addr.sin_family = AF_INET;
    this->m_addr.sin_addr.s_addr = addr;
    this->m_addr.sin_port = port;
}

TCPServer::~TCPServer() {
    if (this->m_started) {
        this->stop();
    }
}

void TCPServer::start() throw () {
    socklen_t socket_len = sizeof(sockaddr_in);
    if (::bind(this->m_socket_fd, (sockaddr *) &this->m_addr, socket_len)) {
        throw std::runtime_error("Bind failed");
    }
    this->set_nonblocking(this->m_socket_fd);
    if (::listen(this->m_socket_fd, MAX_QUEUED_REQUESTS)) {
        throw std::runtime_error("Listen failed");
    }

    epoll_event ev;
    auto events = new epoll_event[MAX_EVENTS];
    const auto epoll_fd  = epoll_create1(0);
    if (epoll_fd < 0) {
        throw std::runtime_error("Epoll instance created failed");
    }
    ev.events = EPOLLIN;
    ev.data.fd = this->m_socket_fd;
    if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->m_socket_fd, &ev) < 0) {
        throw std::runtime_error("Add socket into epoll instance failed");
    }

    this->m_started = true;
    std::cout << "server: waiting for connections...\n";
    sockaddr_in client_addr;
    auto new_socket = 0;
    auto buffer = new uint8_t[BUFFER_SIZE];
    while (1) {
        auto n_fds = ::epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (auto i = 0; i < n_fds; ++i) {
            const auto fd = events[i].data.fd;
            if ((events[i].events & EPOLLERR) || (!(events[i].events & EPOLLIN))) {
                ::close(fd);
                continue;
            } else if (fd == this->m_socket_fd) {
                auto new_socket = ::accept(this->m_socket_fd, (sockaddr *) &client_addr, &socket_len);
                if (new_socket < 0) {
                    std::cerr << "Accept failed\n";
                    continue;
                }
                try {
                    this->set_nonblocking(new_socket);
                } catch(const std::exception& e) {
                    std::cerr << e.what() << '\n';
                    continue;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_socket;
                if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev) < 0) {
                    std::cerr << "Add incoming socket into epoll instance failed\n";
                    continue;
                }
                continue;
            }
            std::cout << "server: got connection from " << this->to_ip_addr(client_addr.sin_addr.s_addr) << '\n';
            this->process(fd);
            ::shutdown(fd, SHUT_RDWR);
            while (::recv(fd, buffer, BUFFER_SIZE, 0) > 0) ;
            ::close(fd);
        }
    }
    delete events;
    delete buffer;
}

void TCPServer::stop() throw () {
    if (!this->m_started) {
        throw std::runtime_error("Server already stoped");
    }
    std::cout << "server: stop...\n";
    ::shutdown(this->m_socket_fd, SHUT_RDWR);
    auto buffer = new uint8_t[BUFFER_SIZE];
    while (::recv(this->m_socket_fd, buffer, BUFFER_SIZE, 0) > 0) ;
    delete buffer;
    ::close(this->m_socket_fd);
    this->m_socket_fd = 0;
    this->m_started = false;
}

std::string TCPServer::to_ip_addr(const uint32_t iaddr) const {
    auto paddr = new char[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &iaddr, paddr, INET_ADDRSTRLEN);
    auto result = std::string(paddr);
    delete paddr;
    return result;
}

std::vector<uint8_t> TCPServer::handle(const std::vector<uint8_t> &msg) throw () {
    throw std::runtime_error("Not supported");
}

void TCPServer::process(const int socket_fd) throw () {
    auto buffer = new uint8_t[BUFFER_SIZE] ;
    const auto recv_size = ::recv(socket_fd, buffer, BUFFER_SIZE, 0);
    const auto msg = std::vector<uint8_t>(buffer, buffer + recv_size);
    delete buffer;
    auto response = this->handle(msg);
    ::send(socket_fd, response.data(), response.size(), 0);
}

int TCPServer::set_nonblocking(const int fd) const throw() {
    auto flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error("Get file status failed");
    }
    flags |= O_NONBLOCK ;
    if (::fcntl (fd, F_SETFL, flags) < 0) {
        throw std::runtime_error("Set file status failed");
    }
    return fd;
}

DHCPServer::DHCPServer(const in_addr_t addr, const in_port_t port) throw () :
    TCPServer(addr, port),
    mt(std::random_device()()),
    dist(std::uniform_int_distribution<uint32_t>(0x000000U, 0xffffffU)) {}

DHCPServer::~DHCPServer() {}

std::vector<uint8_t> DHCPServer::handle(const std::vector<uint8_t> &msg) throw () {
    const auto message_len = sizeof(SimpleDHCPMessage);
    if (msg.size() < message_len) {
        return std::vector<uint8_t>(message_len, 0x00);
    }

    auto dhcp_message = new SimpleDHCPMessage();
    std::memcpy(dhcp_message, msg.data(), message_len);
    auto xid = ::ntohl(dhcp_message->xid), iaddr = ::ntohl(dhcp_message->iaddr);

    // DHCPOFFER
    if (!iaddr) {
        std::cout << "Received the following Transaction ID from client: " << xid << '\n';
        iaddr = 0xff & xid;
        iaddr |= this->dist(this->mt) << 8;
        dhcp_message->iaddr |=  ::htonl(iaddr);
        xid = 0xff & this->dist(this->mt);
        dhcp_message->xid = ::htonl(xid);
        std::cout << "Sending the following to Client:\n"
            << "IP address: " << this->to_ip_addr(dhcp_message->iaddr) << '\n'
            << "Transaction ID: " << xid << '\n';
        auto response_msg = std::vector<uint8_t>((uint8_t *) dhcp_message, ((uint8_t *) dhcp_message + message_len));
        delete dhcp_message;
        return response_msg;
    }

    // DHCPACK
    std::cout << "Received the following request:\n"
        << "Transaction ID: " << xid << '\n'
        << "Sending following acknowledgment:\n";
    xid = 0xff & this->dist(this->mt);
    dhcp_message->xid = ::htonl(xid);
    std::cout << "IP address: " << this->to_ip_addr(dhcp_message->iaddr) << '\n'
        << "Transaction ID: " << xid << '\n';
    auto response_msg = std::vector<uint8_t>((uint8_t *) dhcp_message, ((uint8_t *) dhcp_message + message_len));
    delete dhcp_message;
    return response_msg;
}

int main(int argc, char *argv[]) {
    std::cout << std::unitbuf;
    auto server = DHCPServer(ADDR, htons(PORT));
    try {
        server.start();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    server.stop();
    return EXIT_SUCCESS;
}
