#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>

#ifndef __EE669_ARG_PARSER
#define __EE669_ARG_PARSER

class ArgParser {
private:
    std::unordered_set<std::string> args;
    std::unordered_set<std::string> boolArgs;

public:
    ArgParser();
    virtual ~ArgParser();

    std::unordered_map<std::string, std::string> parse(int argc, char *argv[]) const;
    ArgParser *addArgument(const std::string arg);
    ArgParser *addBoolArgument(const std::string arg);
};

#endif
