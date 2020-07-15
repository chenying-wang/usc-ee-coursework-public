#include <iostream>
#include <string>
#include <utility>

#include <netinet/in.h>

#ifndef __EE450_UTILS
#define __EE450_UTILS

class Utils final {
private:
    Utils();
    virtual ~Utils();

public:
    static int set_socket_reuseaddr(const int socket_fd);
    static int set_socket_nonblocking(const int fd);
    static int set_tcp_nodelay(const int socket_fd);
    static std::string to_ip_addr(const in_addr_t iaddr);
    static std::pair<uint8_t *, size_t> serialize(const std::vector<std::vector<uint8_t>> &msg);
    static std::string get_filename(const std::string &full_filename);
    static std::pair<std::string, in_port_t> process_header(const std::vector<uint8_t> &header);
};

#endif
