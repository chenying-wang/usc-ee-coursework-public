#include <iostream>
#include <vector>

#include <netinet/in.h>

#ifndef __EE450_UDP_CLIENT
#define __EE450_UDP_CLIENT

class UDPClient {
private:
    int m_socket_fd;
    sockaddr_in m_addr;

    UDPClient();

	UDPClient &operator=(const UDPClient &arg);

public:
    UDPClient(const UDPClient &arg);
    virtual ~UDPClient();

    static UDPClient &instance();

    void request(const in_addr_t iaddr, const in_port_t port, const std::vector<uint8_t> &msg) const;
    std::pair<in_addr_t, in_port_t> get_info() const;
};

#endif
