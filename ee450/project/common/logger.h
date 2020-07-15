#include <iostream>
#include <string>
#include <chrono>

#include <pthread.h>

#ifndef __EE450_LOGGER
#define __EE450_LOGGER

class Logger {
private:
    const static std::string ERROR;
    const static std::string INFO;
    const static std::string DEBUG;

    const static std::chrono::time_point<std::chrono::high_resolution_clock> start;
    static pthread_mutex_t mtx;

    Logger();
    virtual ~Logger();

    static std::string format(const std::string &msg, const std::string &level);

public:
    static void init();
    static void error(const std::string &msg);
    static void info(const std::string &msg);
    static void debug(const std::string &msg);
};

#endif
