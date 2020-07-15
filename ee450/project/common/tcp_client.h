#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <functional>
#include <future>

#include <pthread.h>
#include <netinet/in.h>

#ifndef __EE450_TCP_CLIENT
#define __EE450_TCP_CLIENT

class TCPClient final {
private:
    struct __request {
        int fd;
        in_addr_t iaddr;
        in_port_t port;
        std::vector<std::vector<uint8_t>> msg_bytes;
        std::string client_name;
        std::string on_connect_msg;
        std::vector<std::string> on_send_msg;
        std::string on_finish_msg;
        size_t buffer_size;
    };

    bool m_try_stop;
    std::queue<TCPClient::__request *> m_requests;
    std::unordered_map<int, std::promise<std::vector<std::vector<uint8_t>>>> m_result;
    pthread_mutex_t m_requests_mtx;
    pthread_mutex_t m_results_mtx;

    std::vector<uint8_t *> m_stacks;

    TCPClient();

	TCPClient &operator=(const TCPClient &arg);

    int worker_process();

public:
    TCPClient(const TCPClient &arg);
    virtual ~TCPClient();

    static TCPClient &instance();

    void start(const uint n_workers);
    void stop();
    std::future<std::vector<std::vector<uint8_t>>> request(const in_addr_t iaddr, const in_port_t port,
                                                           const std::vector<std::vector<uint8_t>> &msg,
                                                           const std::string client_name,
                                                           const std::string on_connect_msg,
                                                           const std::vector<std::string> &on_send_msg,
                                                           const std::string on_finish_msg,
                                                           const size_t buffer_size);
};

#endif
