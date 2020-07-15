#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include <pthread.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "config.h"

#ifndef __EE450_TCP_SERVER
#define __EE450_TCP_SERVER

class TCPServer {
private:
    const uint m_buffer_size;
    const uint m_max_queued_requests;
    const uint m_max_events;

    bool m_started;
    int m_socket_fd;
    sockaddr_in m_addr;

    int m_epoll_fd;

    std::unordered_map<int, std::vector<std::vector<uint8_t>>> m_ctx;
    std::queue<int> m_accept_fds;
    std::queue<int> m_finish_fds;
    std::unordered_set<int> m_wait_fds;
    pthread_mutex_t m_ctx_mtx;
    pthread_mutex_t m_fds_mtx;

    std::vector<uint8_t *> m_stacks;

    int master_process();
    int worker_process();

    int process(const int socket_fd);

protected:
    bool m_try_stop;

    virtual std::vector<std::vector<uint8_t>> handle(const std::vector<std::vector<uint8_t>> &msg);
    virtual void post_send();
    std::pair<in_addr_t, in_port_t> get_server_info() const;

public:
    TCPServer(const in_addr_t iaddr, const in_port_t port,
              const uint buffer_size = BUFFER_SIZE,
              const uint max_queued_requests = MAX_QUEUED_REQUESTS,
              const uint max_events = MAX_EVENTS);
    virtual ~TCPServer();

    void start(const uint n_worker_processes);
    void stop();
};

#endif
