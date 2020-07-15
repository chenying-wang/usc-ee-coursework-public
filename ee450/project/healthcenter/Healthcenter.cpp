#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <utility>
#include <cstring>

#include <sys/wait.h>
#include <netinet/in.h>

#include "Healthcenter.h"
#include "common/config.h"
#include "common/tcp_server.h"
#include "common/udp_client.h"
#include "common/logger.h"
#include "common/utils.h"

#define HOSPITAL_INPUT_DELIMITER ('#')
#define HOSPITAL_INPUT_RANGE_DELIMITER (',')
#define STUDENT_INPUT_DELIMITER (':')

Healthcenter::Healthcenter(const in_addr_t iaddr, const in_port_t port) :
    TCPServer(iaddr, port) {}

Healthcenter::~Healthcenter() {}

void Healthcenter::start(const uint n_worker_processes) {
    this->m_assigned = false;
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_setrobust(&att, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&this->m_mtx, &att);
    pthread_mutex_init(&this->m_hospital_ctx_mtx, &att);
    pthread_mutex_init(&this->m_student_ctx_mtx, &att);
    pthread_mutex_init(&this->m_department_ctx_mtx, &att);
    pthread_mutex_init(&this->m_student_info_ctx_mtx, &att);

    const auto [iaddr, port] = this->get_server_info();
    Logger::info("The health center has TCP port " + std::to_string(ntohs(port)) +
            " and IP address " + Utils::to_ip_addr(iaddr));
    TCPServer::start(n_worker_processes);
}

std::vector<std::vector<uint8_t>> Healthcenter::handle(const std::vector<std::vector<uint8_t>> &msg) {
    uint status;
    auto change_of_status = false;
    {
        pthread_mutex_lock(&this->m_mtx);
        status = this->m_status;
        ++this->m_count;
        if (this->m_count == 3) {
            change_of_status = true;
            this->m_status = PHASE_2;
        } else if (this->m_count == 8) {
            change_of_status = true;
            this->m_status = TERMINATED;
        }
        pthread_mutex_unlock(&this->m_mtx);
    }
    if (status == PHASE_1) {
        const auto &resp = this->handle_phase_1(msg);
        if (change_of_status) {
            Logger::info("End of Phase 1 for the health center");
            const auto [iaddr, port] = this->get_server_info();
            Logger::info("The health center has TCP port " + std::to_string(ntohs(port)) +
                    " and IP address " + Utils::to_ip_addr(iaddr));
        }
        return resp;
    } else if (status == PHASE_2) {
        const auto &resp = this->handle_phase_2(msg);
        return resp;
    }
    return std::vector<std::vector<uint8_t>>();
}

void Healthcenter::post_send() {
    if (this->m_status == TERMINATED) {
        this->assign_student();
    }
}

std::vector<std::vector<uint8_t>> Healthcenter::handle_phase_1(const std::vector<std::vector<uint8_t>> &msg) {
    const auto msg_size = msg.size();
    if (!msg_size) {
        throw std::runtime_error("Format error");
    }
    const auto &[hospital_name, port] = Utils::process_header(msg.front());
    {
        pthread_mutex_lock(&this->m_hospital_ctx_mtx);
        this->m_hospital_ctx.insert(std::make_pair(hospital_name, port));
        pthread_mutex_unlock(&this->m_hospital_ctx_mtx);
    }
    for (auto i = 1u; i < msg_size; ++i) {
        const auto line = msg.at(i);
        const auto delimiter_idx = std::find(line.cbegin(), line.cend(), HOSPITAL_INPUT_DELIMITER);
        if (delimiter_idx == line.cend()) {
            throw std::runtime_error("Format error: " + std::string(line.cbegin(), line.cend()));
        }
        const auto department_name = std::string(line.cbegin(), delimiter_idx);
        const auto range_delimiter_idx = std::find(delimiter_idx + 1, line.cend(), HOSPITAL_INPUT_RANGE_DELIMITER);
        if (range_delimiter_idx == line.cend()) {
            throw std::runtime_error("Format error: " + std::string(line.cbegin(), line.cend()));
        }
        auto begin = -1, end = -1;
        try {
            begin = std::stoi(std::string(delimiter_idx + 1, range_delimiter_idx));
            end = std::stoi(std::string(range_delimiter_idx + 1, line.cend()));
        } catch (std::exception &e) {
            Logger::error(e.what());
            throw std::runtime_error("Format error: " + std::string(line.cbegin(), line.cend()));
        }
        if (begin < 0 || end < 0 || begin > end) {
            throw std::runtime_error("Format error: " + std::string(line.cbegin(), line.cend()));
        }

        {
            pthread_mutex_lock(&this->m_department_ctx_mtx);
            if (!this->m_department_ctx.count(department_name)) {
                this->m_department_ctx.insert(std::make_pair(department_name, std::list<std::tuple<std::string, int, int>>()));
            }
            this->m_department_ctx.at(department_name).push_back(std::make_tuple(hospital_name, begin, end));
            pthread_mutex_unlock(&this->m_department_ctx_mtx);
        }
    }
    Logger::info("Received the department list from " + hospital_name);

    std::string r = "Okay! Phase 1, " + hospital_name;
    auto resp = std::vector<std::vector<uint8_t>>();
    resp.push_back(std::vector<uint8_t>(r.cbegin(), r.cend()));
    return resp;
}

std::vector<std::vector<uint8_t>> Healthcenter::handle_phase_2(const std::vector<std::vector<uint8_t>> &msg) {
    const auto msg_size = msg.size();
    if (!msg_size) {
        throw std::runtime_error("Format error");
    }
    const auto &[student_name, port] = Utils::process_header(msg.front());
    {
        pthread_mutex_lock(&this->m_student_ctx_mtx);
        this->m_student_ctx.insert(std::make_pair(student_name, port));
        pthread_mutex_unlock(&this->m_student_ctx_mtx);
    }
    Logger::info("health center receive the application from " + student_name);
    if (msg_size < 2) {
        return std::vector<std::vector<uint8_t>>({std::vector<uint8_t>({0x0})});
    }

    auto symptom = -1;
    auto interests = std::list<std::string>();
    for (auto i = 1u; i < msg_size; ++i) {
        const auto line = msg.at(i);
        const auto delimiter_idx = std::find(line.cbegin(), line.cend(), STUDENT_INPUT_DELIMITER);
        if (delimiter_idx == line.cend()) {
            throw std::runtime_error("Format error: " + std::string(line.cbegin(), line.cend()));
        }
        const auto info = std::string(delimiter_idx + 1, line.cend());
        if (i != 1) {
            interests.push_back(info);
            continue;
        }
        try {
            symptom = std::stoi(info);
        } catch (std::exception &e) {
            Logger::error(e.what());
            throw std::runtime_error("Format error: " + std::string(line.cbegin(), line.cend()));
        }
        if (symptom < 0) {
            throw std::runtime_error("Format error: " + std::string(line.cbegin(), line.cend()));
        }
    }
    if (symptom < 0 || interests.empty()) {
        return std::vector<std::vector<uint8_t>>({std::vector<uint8_t>({0x0})});
    }

    auto valid = false;
    for (const auto &interest : interests) {
        if (this->m_department_ctx.count(interest)) {
            valid = true;
            break;
        }
    }
    if (!valid) {
        return std::vector<std::vector<uint8_t>>({std::vector<uint8_t>({0x0})});
    }
    {
        pthread_mutex_lock(&this->m_student_info_ctx_mtx);
        this->m_student_info_ctx.insert(std::make_pair(student_name, std::make_pair(symptom, interests)));
        pthread_mutex_unlock(&this->m_student_info_ctx_mtx);
    }
    return std::vector<std::vector<uint8_t>>({std::vector<uint8_t>({0x1})});
}

void Healthcenter::assign_student() {
    pthread_mutex_lock(&this->m_mtx);
    if (this->m_assigned) {
        pthread_mutex_unlock(&this->m_mtx);
        return;
    }
    auto result = std::unordered_map<std::string, std::pair<std::string, std::string>>();
    for (const auto &[student_name, student_info] : this->m_student_info_ctx) {
        const auto symptom = student_info.first;
        const auto &interests = student_info.second;
        bool ok = false;
        for (const auto &interest : interests) {
            if (!this->m_department_ctx.count(interest)) {
                continue;
            }
            for (const auto &[hospital_name, begin, end] : this->m_department_ctx.at(interest)) {
                if (symptom >= begin && symptom <= end) {
                    result.insert(std::make_pair(student_name, std::make_pair(interest, hospital_name)));
                    ok = true;
                    break;
                }
            }
            if (ok) {
                break;
            }
        }
    }

    auto &udp_client = UDPClient::instance();
    const auto [iaddr, port] = udp_client.get_info();
    Logger::info("The health center has UDP port " + std::to_string(ntohs(port)) +
            " and IP address " + Utils::to_ip_addr(iaddr) + " for Phase 2");
    ::nanosleep((const timespec[]){{0, 100 * 1000000L}}, nullptr);
    for (const auto &[student_name, port] : this->m_student_ctx) {
        auto student_msg = std::string();
        if (result.count(student_name)) {
            const auto &[department_name, hospital_name] = result.at(student_name);
            student_msg = "Accept:" + department_name + '#' + hospital_name;
            const auto symptom = this->m_student_info_ctx.at(student_name).first;
            const auto hospital_msg = student_name + '#' + std::to_string(symptom) + '#' + department_name;
            udp_client.request(HOSPITAL_SERV_ADDR, this->m_hospital_ctx.at(hospital_name),
                std::vector<uint8_t>(hospital_msg.cbegin(), hospital_msg.cend()));
            Logger::info("The health center has sent one admitted student to " + hospital_name);
        } else {
            student_msg = "Reject";
        }
        udp_client.request(STUDENT_SERV_ADDR, this->m_student_ctx.at(student_name),
                std::vector<uint8_t>(student_msg.cbegin(), student_msg.cend()));
        Logger::info("The health center has sent the application result to " + student_name);
    }
    this->m_assigned = true;
    pthread_mutex_unlock(&this->m_mtx);
}

int main(int argc, char *argv[]) {
    Logger::init();
    auto healthcenter = Healthcenter(HEALTHCENTER_SERV_ADDR, HEALTHCENTER_SERV_PORT);
    try {
        healthcenter.start(1);
    } catch (const std::exception  &e) {
        Logger::error(e.what());
        return EXIT_FAILURE;
    }
    healthcenter.stop();
    while (wait(nullptr) > 0) ;
    return EXIT_SUCCESS;
}
