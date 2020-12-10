#include <iostream>
#include <bitset>
#include <cstddef>
#include <climits>

#ifndef __EE669_CODE
#define __EE669_CODE

#define MAX_CODE_SIZE(T) ((1 << CHAR_BIT * sizeof(T)) - 1)

template<typename T>
class Code {
public:
    const uint64_t frequency;
    const T symbol;
    std::bitset<MAX_CODE_SIZE(T)> code;
    uint32_t codeLen;

    Code() :
        frequency(0UL), symbol(T(0)), codeLen(0u) {}
    Code(const uint64_t frequency, const T symbol = T(0)) :
        frequency(frequency), symbol(symbol), codeLen(0u) {}

};

#endif
