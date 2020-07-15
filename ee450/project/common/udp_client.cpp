#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>

#include "udp_client.h"
#include "common/logger.h"

UDPClient::UDPClient() {
    if ((this->m_socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw std::runtime_error("Socket created failed");
    }
    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(0);
    socklen_t len = sizeof(servaddr);
    auto msg = new uint8_t[1]{'\0'};
    ::sendto(this->m_socket_fd, msg, 1, 0, (const sockaddr *) &servaddr, len);
    delete[] msg;
    if (::getsockname(this->m_socket_fd, (sockaddr *) &this->m_addr, &len) < 0) {
        throw std::runtime_error("Socket " + std::to_string(this->m_socket_fd) + " getsockname failed");
    }
    this->m_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

UDPClient::~UDPClient() {
    ::close(this->m_socket_fd);
}

UDPClient &UDPClient::operator=(const UDPClient &) {
    return instance();
}

UDPClient::UDPClient(const UDPClient &) {}

UDPClient &UDPClient::instance() {
    static UDPClient instance;
    return instance;
}

void UDPClient::request(const in_addr_t iaddr, const in_port_t port, const std::vector<uint8_t> &msg) const {
    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = iaddr;
    servaddr.sin_port = port;
    if (::sendto(this->m_socket_fd, msg.data(), msg.size(), 0, (const sockaddr *) &servaddr,
            sizeof(servaddr)) < 0) {
        Logger::error("Socket " + std::to_string(this->m_socket_fd) + " send failed");
    }
}

std::pair<in_addr_t, in_port_t> UDPClient::get_info() const {
    return std::make_pair(this->m_addr.sin_addr.s_addr, this->m_addr.sin_port);
}
