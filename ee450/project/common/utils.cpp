#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <cstring>

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "utils.h"
#include "config.h"

Utils::Utils() {}

Utils::~Utils() {}

int Utils::set_socket_reuseaddr(const int socket_fd) {
    const int opt = 1;
    if (::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Set socket option failed");
    }
    return socket_fd;
}

int Utils::set_socket_nonblocking(const int socket_fd) {
    auto flags = ::fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error("Get file status failed");
    }
    flags |= O_NONBLOCK ;
    if (::fcntl (socket_fd, F_SETFL, flags) < 0) {
        throw std::runtime_error("Set file status failed");
    }
    return socket_fd;
}

int Utils::set_tcp_nodelay(const int socket_fd) {
    const int opt = 1;
    if (::setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
        throw std::runtime_error("Set tcp option failed");
    }
    return socket_fd;
}

std::string Utils::to_ip_addr(const in_addr_t iaddr) {
    auto paddr = new char[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &iaddr, paddr, INET_ADDRSTRLEN);
    auto result = std::string(paddr);
    delete[] paddr;
    return result;
}

std::pair<uint8_t *, size_t> Utils::serialize(const std::vector<std::vector<uint8_t>> &msg) {
    if (msg.empty()) {
        return std::make_pair(new uint8_t[1]{MSG_DELIMITER}, 1UL);
    }
    auto size = 1UL;
    for (const auto &line : msg) {
        size += line.size() + 1;
    }
    auto bytes = new uint8_t[size], i = bytes;
    bytes[size - 1] = MSG_DELIMITER;
    for (const auto &line : msg) {
        std::memcpy(i, line.data(), line.size());
        i += line.size();
        *i++ = MSG_DELIMITER;
    }
    return std::make_pair(bytes, size);
}

std::string Utils::get_filename(const std::string &full_filename) {
    auto pos1 = full_filename.rfind('/');
    if (pos1 == std::string::npos) {
        pos1 = 0;
    } else {
        ++pos1;
    }
    auto pos2 = full_filename.rfind('.');
    if (pos2 == std::string::npos) {
        pos2 = full_filename.size();
    }
    return std::string(full_filename.data() + pos1, full_filename.data() + pos2);
}

std::pair<std::string, in_port_t> Utils::process_header(const std::vector<uint8_t> &header) {
    const auto pos = std::find(header.cbegin(), header.cend(), ':');
    const auto hospital_name = std::string(header.cbegin(), pos);
    const in_port_t port = *(pos + 1) << 8 | *(pos + 2);
    return std::make_pair(hospital_name, port);
}
