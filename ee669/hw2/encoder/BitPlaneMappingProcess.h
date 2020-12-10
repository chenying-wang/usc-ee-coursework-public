#ifndef __EE669_BIT_PLANE_PROCESS
#define __EE669_BIT_PLANE_PROCESS

#include <iostream>
#include <vector>

#include "common/ByteStreamProcess.h"

template<typename T>
class BitPlaneMappingProcess : public ByteStreamProcess<T> {
private:
    std::vector<T> process0(std::vector<T> &input);

public:
    BitPlaneMappingProcess(const bool procSwitch = true);
    virtual ~BitPlaneMappingProcess();
};

template<typename T>
BitPlaneMappingProcess<T>::BitPlaneMappingProcess(const bool procSwitch) : ByteStreamProcess<T>(procSwitch) {}

template<typename T>
BitPlaneMappingProcess<T>::~BitPlaneMappingProcess() {}

template<typename T>
std::vector<T>
BitPlaneMappingProcess<T>::process0(std::vector<T> &input) {
    const auto size = input.size();
    if (size < 2) {
        return input;
    }
    std::vector<T> result(size, T(0));
    const auto width = CHAR_BIT * sizeof(T);
    const auto initMask = T(1) << (width - 1);
    auto pos = initMask;
    auto idx = 0UL;
    for (auto mask = initMask; mask != T(0); mask >>= 1) {
        for (const T &byte : input) {
            if ((byte & mask) != T(0)) {
                result[idx] |= pos;
            }
            pos >>= 1;
            if (pos == T(0)) {
                pos = initMask;
                ++idx;
            }
        }
    }
    input = result;
    result.clear();
    return input;
}

#endif
