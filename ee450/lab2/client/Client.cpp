#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT (4034)
#define BUFFER_SIZE (256)
#define TRANSACTION_ID (34)


#ifndef __EE450_TCP_CLIENT
#define __EE450_TCP_CLIENT

class TCPClient {
public:
    TCPClient();
    virtual ~TCPClient();

    std::vector<uint8_t> request(const in_addr_t addr, const in_port_t port, const std::vector<uint8_t> &msg) const throw ();
};

#endif

#ifndef __EE450_DHCP_CLIENT
#define __EE450_DHCP_CLIENT

class DHCPClient {
private:
    TCPClient tcp_client;
    std::mt19937 mt;
    std::uniform_int_distribution<uint16_t> dist;

    std::string to_ip_addr(const uint32_t iaddr) const;

public:
    DHCPClient();
    virtual ~DHCPClient();

    uint32_t request(const in_addr_t addr, const in_port_t port) throw ();
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

TCPClient::TCPClient() {}

TCPClient::~TCPClient() {}

std::vector<uint8_t> TCPClient::request(const in_addr_t addr, const in_port_t port, const std::vector<uint8_t> &msg) const throw () {
    int socket_fd;
    if ((socket_fd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Socket created failed");
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = addr;
    serv_addr.sin_port = port;

    if (::connect(socket_fd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Connection failed");
    }

    ::send(socket_fd, msg.data(), msg.size(), 0);

    auto buffer = new uint8_t[BUFFER_SIZE];
    const auto recv_size = ::recv(socket_fd, buffer, BUFFER_SIZE, 0);
    auto result = std::vector<uint8_t>(buffer, buffer + recv_size);
    ::shutdown(socket_fd, SHUT_RDWR);
    while (::recv(socket_fd, buffer, BUFFER_SIZE, 0) > 0) ;
    ::close(socket_fd);
    delete buffer;
    return result;
}

DHCPClient::DHCPClient() :
    mt(std::random_device()()),
    dist(std::uniform_int_distribution<uint16_t>(0x00U, 0xffU)) {}

DHCPClient::~DHCPClient() {}

uint32_t DHCPClient::request(const in_addr_t addr, const in_port_t port) throw () {
    const auto message_len = sizeof(SimpleDHCPMessage);
    // DHCPDISCOVER
    auto dhcp_message = new SimpleDHCPMessage();
    dhcp_message->xid = ::htonl(TRANSACTION_ID);
    dhcp_message->iaddr = ::htonl(0x0);
    auto discover_msg = std::vector<uint8_t>((uint8_t *) dhcp_message, (uint8_t *) dhcp_message + message_len);
    std::cout << "client: connecting to " << this->to_ip_addr(addr) << '\n'
        << "Sending the following Transaction ID to server: " << TRANSACTION_ID << '\n';
    auto offer_msg = this->tcp_client.request(addr, port, discover_msg);
    discover_msg.clear();

    std::memcpy(dhcp_message, offer_msg.data(), message_len);
    auto xid = ::ntohl(dhcp_message->xid);
    auto paddr = this->to_ip_addr(dhcp_message->iaddr);
    std::cout << "Received the following:\n"
        << "IP address: " << paddr << '\n'
        << "Transaction ID: " << xid << '\n';

    // DHCPREQUEST
    std::memcpy(dhcp_message, offer_msg.data(), message_len);
    xid = this->dist(this->mt);
    dhcp_message->xid = ::htonl(xid);
    std::cout << "Formally requesting the following server:\n"
        << "IP address: " << paddr << '\n'
        << "Transaction ID: " << xid << '\n';
    auto request_msg = std::vector<uint8_t>((uint8_t *) dhcp_message, (uint8_t *) dhcp_message + message_len);
    auto ack_msg = this->tcp_client.request(addr, port, request_msg);
    request_msg.clear();

    std::memcpy(dhcp_message, ack_msg.data(), message_len);
    ack_msg.clear();
    if (!::ntohl(dhcp_message->iaddr)) {
        delete dhcp_message;
        throw std::runtime_error("Failed to request address");
    }
    xid = ::ntohl(dhcp_message->xid);
    paddr = this->to_ip_addr(dhcp_message->iaddr);
    std::cout << "Officially connected to IP Address: " << paddr << '\n'
        << "client: received " << xid << '\n';
    const auto result = dhcp_message->iaddr;
    delete dhcp_message;
    return result;
}

std::string DHCPClient::to_ip_addr(const uint32_t iaddr) const {
    auto paddr = new char[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &iaddr, paddr, INET_ADDRSTRLEN);
    auto result = std::string(paddr);
    delete paddr;
    return result;
}

int main(int argc, char *argv[]) {
    in_addr_t serv_addr = htonl(INADDR_LOOPBACK);
    if (argc > 1) {
        const auto host = ::gethostbyname(argv[1]);
        if (host != NULL && host->h_addrtype == AF_INET && host->h_length > 0) {
            std::memcpy(&serv_addr, host->h_addr_list[0], sizeof(serv_addr));
        } else {
            std::cerr << "Failed to get host IP address, use 127.0.0.1 instead\n";
        }
    } else {
        std::cerr << "Missing argument of hostname, use 127.0.0.1 by default\n";
    }
    std::cout << std::unitbuf;
    auto client = DHCPClient();
    try {
        std::cout << "Client Start...\n";
        const auto response = client.request(serv_addr, htons(SERVER_PORT));
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    return EXIT_SUCCESS;
}
