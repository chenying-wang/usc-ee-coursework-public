#include <iostream>
#include <vector>

#include <netinet/in.h>

#include "config.h"

#ifndef __EE450_UDP_SERVER
#define __EE450_UDP_SERVER

class UDPServer {
private:
    const uint m_buffer_size;

    bool m_started;
    bool m_try_stop;
    int m_socket_fd;
    sockaddr_in m_addr;

protected:
    virtual void handle(const std::vector<uint8_t> &msg);
    std::pair<in_addr_t, in_port_t> get_server_info() const;

public:
    UDPServer(const in_addr_t iaddr, const in_port_t port, const uint buffer_size = BUFFER_SIZE);
    virtual ~UDPServer();

    void start();
    void stop();
    void try_stop();
};

#endif
