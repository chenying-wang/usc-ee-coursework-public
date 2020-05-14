/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
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
    char *optDefaultCpy = new char[strlen(optDefault)];
    memcpy(optDefaultCpy, optDefault, sizeof(char) * strlen(optDefault));
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
