#ifndef __EE669_ZIGZAG_SCAN_PROCESS
#define __EE669_ZIGZAG_SCAN_PROCESS

#include <iostream>
#include <vector>

#include "common/Matrix.h"

template<typename T>
class ScanOrderProcess : public ByteStreamProcess<T>  {
private:
    const uint64_t width;
    const uint64_t height;
    typename Matrix<T>::iterator *it;

    std::vector<T> process0(std::vector<T> &input);

public:
    ScanOrderProcess(typename Matrix<T>::iterator *it,
            const uint64_t width, const uint64_t height, const bool procSwitch = true);
    virtual ~ScanOrderProcess();
};

template<typename T>
ScanOrderProcess<T>::ScanOrderProcess(typename Matrix<T>::iterator *it,
    const uint64_t width, const uint64_t height, const bool procSwitch) :
    ByteStreamProcess<T>(procSwitch), it(it), width(width), height(height) {}

template<typename T>
ScanOrderProcess<T>::~ScanOrderProcess() {
    delete it;
}

template<typename T>
std::vector<T>
ScanOrderProcess<T>::process0(std::vector<T> &input) {
    auto matrix = new Matrix<T>(this->width, this->height);
    matrix->load(input.cbegin(), input.cend());
    std::vector<T> result;
    result.reserve(input.size());

    this->it->set(matrix)->forEach([&result] (T &data) {
        result.push_back(data);
    });
    delete matrix;
    return result;
}

#endif
