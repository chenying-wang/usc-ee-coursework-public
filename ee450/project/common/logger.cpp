#include <iostream>
#include <string>
#include <chrono>

#include <unistd.h>
#include <pthread.h>

#include "logger.h"

Logger::Logger() {}

Logger::~Logger() {}

const std::string Logger::ERROR = std::string("ERROR");
const std::string Logger::INFO = std::string("INFO");
const std::string Logger::DEBUG = std::string("DEBUG");

const std::chrono::time_point<std::chrono::high_resolution_clock> Logger::start = std::chrono::high_resolution_clock::now();
pthread_mutex_t Logger::mtx;

void Logger::init() {
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_setrobust(&att, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&Logger::mtx, &att);
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
}

void Logger::error(const std::string &msg) {
    const auto log_msg = Logger::format(msg, Logger::ERROR);
    {
        pthread_mutex_lock(&Logger::mtx);
        std::cerr << log_msg;
        pthread_mutex_unlock(&Logger::mtx);
    }
}

void Logger::info(const std::string &msg) {
    const auto log_msg = Logger::format(msg, Logger::INFO);
    {
        pthread_mutex_lock(&Logger::mtx);
        std::cout << log_msg;
        pthread_mutex_unlock(&Logger::mtx);
    }
}

void Logger::debug(const std::string &msg) {
#ifdef __DEBUG
    const auto log_msg = Logger::format(msg, Logger::DEBUG);
    {
        pthread_mutex_lock(&Logger::mtx);
        std::cout << log_msg;
        pthread_mutex_unlock(&Logger::mtx);
    }
#endif
}

std::string Logger::format(const std::string &msg, const std::string &level) {
    auto log_msg = std::string();
#ifdef __DEBUG
    const auto now = std::chrono::high_resolution_clock::now();
    log_msg += '[' + level + "] ";
    log_msg += std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now - Logger::start).count());
    log_msg += "ms p";
    log_msg += std::to_string(getpid());
    log_msg.push_back(' ');
#endif
    log_msg += msg;
    log_msg.push_back('\n');
    return log_msg;
}
