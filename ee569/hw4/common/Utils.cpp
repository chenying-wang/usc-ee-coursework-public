/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <cstring>

#include "Utils.h"

char *Utils::getOption(int argc, char *argv[], const char *option, const char *optDefault) {
    for (auto i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], option) == 0 && ++i < argc) {
            return std::strcpy(new char[1 + std::strlen(argv[i])], argv[i]);
        }
    }

    if (optDefault == nullptr) {
        return nullptr;
    }
    return std::strcpy(new char[1 + std::strlen(optDefault)], optDefault);
}

bool Utils::getBoolOption(int argc, char *argv[], const char *option) {
    for (auto i = 0; i < argc; ++i) {
        if (!std::strcmp(argv[i], option)) {
            return true;
        }
    }
    return false;
}
