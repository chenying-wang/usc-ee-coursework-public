#ifndef __EE669_UTILS
#define __EE669_UTILS

#include <iostream>
#include <vector>
#include <unordered_map>

class Utils {
private:
    Utils();

public:
    virtual ~Utils();

    template<typename T, typename Iterator>
    static std::unordered_map<T, uint64_t> stat(Iterator first, Iterator last);
};

Utils::Utils() {}

Utils::~Utils() {}

template<typename T, typename Iterator>
std::unordered_map<T, uint64_t> Utils::stat(Iterator first, Iterator last) {
    auto count = std::unordered_map<T, uint64_t>();
    while (first != last) {
        ++count[*first];
        ++first;
    }
    return count;
}

#endif
