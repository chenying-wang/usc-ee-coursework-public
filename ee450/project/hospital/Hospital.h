#include <iostream>
#include <vector>
#include <string>

#include <netinet/in.h>

#include "common/tcp_client.h"
#include "common/udp_server.h"

#ifndef __EE450_HOSPITAL
#define __EE450_HOSPITAL

class Hospital : public UDPServer {
private:
    const in_port_t m_recv_port;

    std::vector<std::vector<uint8_t>> m_msg;
    std::vector<std::string> m_on_send_msg;

protected:
    void handle(const std::vector<uint8_t> &msg) override;

public:
    std::string m_name;

    Hospital(const std::string &input_file, const in_addr_t recv_iaddr, const in_port_t recv_port);
    virtual ~Hospital();

    int request(const in_addr_t iaddr, const in_port_t port);
};

#endif
