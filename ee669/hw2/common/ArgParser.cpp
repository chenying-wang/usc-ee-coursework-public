#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "ArgParser.h"

ArgParser::ArgParser() {}

ArgParser::~ArgParser() {}

std::unordered_map<std::string, std::string> ArgParser::parse(int argc, char *argv[]) const {
    std::unordered_map<std::string, std::string> result;
    if (argc < 1 || argv == nullptr) {
        return result;
    }
    for (auto i = argc - 1, j = argc; i >= 0; --i) {
        const auto arg = std::string(argv[i]);
        if (this->args.count(arg)) {
            auto val = j > i + 1 ? std::string(argv[i + 1]) : this->args.at(arg);
            result.insert(std::make_pair(arg, val));
            result[arg] = val;
            j = i;
        } else if (this->boolArgs.count(arg)) {
            result[arg] = std::string();
            j = i;
        }
    }
    for (const auto &[arg, defaultValue] : this->args) {
        if (!result.count(arg)) {
            result[arg] = defaultValue;
        }
    }
    return result;
}

ArgParser *ArgParser::addArgument(const std::string arg, const std::string defaultValue) {
    this->args[arg] = defaultValue;
    return this;
}

ArgParser *ArgParser::addBoolArgument(const std::string arg) {
    this->boolArgs.insert(arg);
    return this;
}
