#include <iostream>
#include <vector>
#include <string>

#include <netinet/in.h>

#include "common/udp_server.h"

#ifndef __EE450_STUDENT
#define __EE450_STUDENT

class Student : public UDPServer {
private:
    const in_port_t m_recv_port;
    std::string m_name;
    std::vector<std::vector<uint8_t>> m_msg;

protected:
    void handle(const std::vector<uint8_t> &msg) override;

public:
    Student(const std::string &input_file, const in_addr_t recv_iaddr, const in_port_t recv_port);
    virtual ~Student();

    int request(const in_addr_t iaddr, const in_port_t port);
};

#endif
