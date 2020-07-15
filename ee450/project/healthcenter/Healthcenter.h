#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <tuple>
#include <string>
#include <mutex>
#include <atomic>

#include <netinet/in.h>

#include "common/tcp_server.h"

#ifndef __EE450_HEALTHCENTER
#define __EE450_HEALTHCENTER

class Healthcenter : public TCPServer {
private:
    std::atomic_uint m_status = PHASE_1;
    std::atomic_uint m_count = 0u;
    std::unordered_map<std::string, in_port_t> m_hospital_ctx;
    std::unordered_map<std::string, in_port_t> m_student_ctx;
    std::unordered_map<std::string, std::list<std::tuple<std::string, int, int>>> m_department_ctx;
    std::unordered_map<std::string, std::pair<int, std::list<std::string>>> m_student_info_ctx;
    pthread_mutex_t m_mtx;
    pthread_mutex_t m_hospital_ctx_mtx;
    pthread_mutex_t m_student_ctx_mtx;
    pthread_mutex_t m_department_ctx_mtx;
    pthread_mutex_t m_student_info_ctx_mtx;

    bool m_assigned;

    std::vector<std::vector<uint8_t>> handle_phase_1(const std::vector<std::vector<uint8_t>> &msg);
    std::vector<std::vector<uint8_t>> handle_phase_2(const std::vector<std::vector<uint8_t>> &msg);
    void assign_student();

protected:
    std::vector<std::vector<uint8_t>> handle(const std::vector<std::vector<uint8_t>> &msg) override;
    void post_send() override;

public:
    Healthcenter(const in_addr_t iaddr, const in_port_t port);
    virtual ~Healthcenter();

    void start(const uint n_worker_processes);
};

#endif
