#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <future>
#include <utility>
#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <sys/wait.h>

#include "Hospital.h"
#include "common/config.h"
#include "common/tcp_client.h"
#include "common/logger.h"
#include "common/utils.h"

Hospital::Hospital(const std::string &input_file, const in_addr_t recv_iaddr, const in_port_t recv_port) :
    UDPServer(recv_iaddr, recv_port),
    m_recv_port(recv_port) {
    std::ifstream fin;
    fin.open(input_file, std::ios::in);
    if (fin.fail()) {
        throw std::runtime_error("Cannot open file " + input_file);
    }
    this->m_name = Utils::get_filename(input_file);

    this->m_msg.push_back(std::vector<uint8_t>(this->m_name.cbegin(), this->m_name.cend()));
    this->m_msg.back().push_back(':');
    this->m_msg.back().push_back(this->m_recv_port >> 8 & 0xff);
    this->m_msg.back().push_back(this->m_recv_port & 0xff);
    auto buffer = new char[BUFFER_SIZE];
    while (fin.good()) {
        fin.getline(buffer, BUFFER_SIZE);
        auto len = std::strlen(buffer);
        if (!len) {
            continue;
        }
        this->m_msg.push_back(std::vector<uint8_t>(buffer, buffer + len));
        auto on_send_msg = this->m_name + " has sent ";
        auto pos = std::memchr(buffer, '#', len);
        if (pos == nullptr) {
            on_send_msg += "<department>";
        } else {
            on_send_msg += std::string(buffer, (char *) pos);
        }
        on_send_msg += " to the health center";
        this->m_on_send_msg.push_back(on_send_msg);
    }
    delete[] buffer;
    fin.close();
}

Hospital::~Hospital() {}

int Hospital::request(const in_addr_t iaddr, const in_port_t port) {
    auto response_future = TCPClient::instance().request(iaddr, port, this->m_msg, this->m_name,
            this->m_name + " is now connected to the health center", this->m_on_send_msg,
            "Updating the health center is done for " + this->m_name, BUFFER_SIZE);
    const auto response = response_future.get();
    Logger::info("End of Phase 1 for " + this->m_name);
    const auto [serv_iaddr, serv_port] = this->get_server_info();
    Logger::info(this->m_name + " has UDP port " + std::to_string(ntohs(serv_port)) +
            " and IP address " + Utils::to_ip_addr(serv_iaddr) + " for Phase 2");
    this->start();
    return EXIT_SUCCESS;
}

void Hospital::handle(const std::vector<uint8_t> &msg) {
    auto pos = std::find(msg.cbegin(), msg.cend(), '#');
    Logger::info(std::string(msg.cbegin(), pos) + " has been admitted to " +
            this->m_name);
}

int main(int argc, char *argv[]) {
    Logger::init();
    if (argc < 2) {
        Logger::error("Please specify the input file");
        return EXIT_FAILURE;
    }

    auto hospitals = std::vector<Hospital *>();
    uint16_t recv_port = 21100 + USC_ID;
    for (auto i = 1; i < argc; ++i) {
        hospitals.push_back(new Hospital(std::string(argv[i]), htonl(INADDR_LOOPBACK), htons(recv_port)));
        recv_port += 100;
    }

    TCPClient::instance().start(3);
    try {
        auto threads = std::vector<std::thread>();
        for (auto hospital : hospitals) {
            threads.push_back(std::thread([hospital]
                { return hospital->request(htonl(INADDR_LOOPBACK), HEALTHCENTER_SERV_PORT); }));
        }
        for (auto &thread : threads) {
            thread.join();
        }
        threads.clear();
    } catch (const std::exception &e) {
        Logger::error(e.what());
        return EXIT_FAILURE;
    }

    for (auto hospital : hospitals) {
        delete hospital;
    }
    TCPClient::instance().stop();
    while (wait(nullptr) > 0) ;
    return EXIT_SUCCESS;
}
