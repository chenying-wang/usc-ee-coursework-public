#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "udp_server.h"
#include "config.h"
#include "logger.h"

UDPServer::UDPServer(const in_addr_t iaddr, const in_port_t port, const uint buffer_size) :
    m_buffer_size(buffer_size) {
    this->m_started = false;
    this->m_try_stop = false;
    if ((this->m_socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw std::runtime_error("Socket created failed");
    }
    this->m_addr.sin_family = AF_INET;
    this->m_addr.sin_addr.s_addr = iaddr;
    this->m_addr.sin_port = port;
    if (::bind(this->m_socket_fd, (sockaddr *) &this->m_addr, sizeof(sockaddr_in)) < 0) {
        throw std::runtime_error("Socket " + std::to_string(this->m_socket_fd) + " bind failed");
    }
}

UDPServer::~UDPServer() {
    this->m_try_stop = true;
    if (this->m_started) {
        this->stop();
    }
}

void UDPServer::start() {
    this->m_started = true;
    auto buffer = new uint8_t[this->m_buffer_size];
    auto recv_size = 0;
    while ((recv_size = ::recv(this->m_socket_fd, buffer, this->m_buffer_size, MSG_WAITALL)) > 0) {
        auto msg = std::vector<uint8_t>(buffer, buffer + MSG_WAITALL);
        try {
            this->handle(msg);
        }
        catch(const std::exception &e) {
            Logger::error("Handle UDP connection (fd: " + std::to_string(this->m_socket_fd) + ") error " + e.what());
        }
        if (!this->m_started) {
            break;
        } else if (this->m_try_stop) {
            this->stop();
            break;
        }
    }
    delete[] buffer;
}

void UDPServer::stop() {
    if (!this->m_started) {
        return;
    }
    ::close(this->m_socket_fd);
    this->m_socket_fd = 0;
    this->m_started = false;
    this->m_try_stop = false;
}

void UDPServer::try_stop() {
    this->m_try_stop = true;
}

void UDPServer::handle(const std::vector<uint8_t> &) {
    throw std::runtime_error("Not supported");
}

std::pair<in_addr_t, in_port_t> UDPServer::get_server_info() const {
    return std::make_pair(this->m_addr.sin_addr.s_addr, this->m_addr.sin_port);
}
