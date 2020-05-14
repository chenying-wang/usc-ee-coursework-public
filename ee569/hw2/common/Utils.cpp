/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <iostream>
#include <cstring>

#include "Utils.h"

char *Utils::getOption(int argc, char *argv[], const char *option, const char *optDefault) {
    for (auto i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], option) == 0 && ++i < argc) {
            return argv[i];
        }
    }

    if (optDefault == nullptr) {
        return nullptr;
    }
    const size_t length = std::strlen(optDefault);
    char *optDefaultCpy = new char[length];
    std::memcpy(optDefaultCpy, optDefault, sizeof(char) * length);
    return optDefaultCpy;
}

bool Utils::getBoolOption(int argc, char *argv[], const char *option) {
    for (auto i = 0; i < argc; ++i) {
        if (!std::strcmp(argv[i], option)) {
            return true;
        }
    }
    return false;
}
