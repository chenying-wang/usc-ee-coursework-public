#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "ArgParser.h"

using ee669::ArgParser;

ArgParser::ArgParser() {}

ArgParser::~ArgParser() {}

std::unordered_map<std::string, std::vector<std::string>> ArgParser::parse(int argc, char *argv[]) const {
    std::unordered_map<std::string, std::vector<std::string>> result;
    if (argc < 1 || argv == nullptr) {
        return result;
    }
    for (auto i = argc - 1, j = argc; i >= 0; --i) {
        const auto arg = std::string(argv[i]);
        if (this->args.count(arg)) {
            if (i + 1 >= j) {
                j = i;
                continue;
            }
            for (auto k = i + 1; k < j; ++k) {
                result[arg].push_back(std::string(argv[k]));
            }
            j = i;
        } else if (this->boolArgs.count(arg)) {
            result[arg] = std::vector<std::string>();
            j = i;
        }
    }
    for (const auto &[arg, defaultValue] : this->args) {
        if (!result.count(arg) && !defaultValue.empty()) {
            result[arg] = std::vector<std::string>({defaultValue});
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
