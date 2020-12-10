#ifndef __EE669_SCAN_PROCESS
#define __EE669_SCAN_PROCESS

#include <iostream>
#include <vector>

#include "common/Matrix.h"
#include "common/StreamProcess.h"

namespace ee669 {
    template<typename T>
    class ScanOrderProcess : public StreamProcess<T>  {
    private:
        const uint64_t width;
        const uint64_t height;
        const bool reverse;
        typename Matrix<T>::iterator *it;

        std::vector<T> process0(std::vector<T> &input);

    public:
        ScanOrderProcess(typename Matrix<T>::iterator *it,
                const uint64_t width, const uint64_t height,
                const bool reverse, const bool procSwitch = true);
        virtual ~ScanOrderProcess();
    };

    template<typename T>
    ScanOrderProcess<T>::ScanOrderProcess(typename Matrix<T>::iterator *it,
        const uint64_t width, const uint64_t height,
        const bool reverse, const bool procSwitch) :
        StreamProcess<T>(procSwitch), width(width), height(height), reverse(reverse), it(it) {}

    template<typename T>
    ScanOrderProcess<T>::~ScanOrderProcess() {
        delete this->it;
    }

    template<typename T>
    std::vector<T>
    ScanOrderProcess<T>::process0(std::vector<T> &input) {
        auto matrix = new Matrix<T>(this->width, this->height);
        if (this->reverse) {
            auto idx = 0UL;
            this->it->set(matrix)->forEach([&input, &idx] (T &data) {
                data = input[idx++];
            });
            std::vector<T> result = matrix->getData();
            delete matrix;
            return result;
        }

        matrix->load(input.cbegin(), input.cend());
        std::vector<T> result;
        result.reserve(input.size());

        this->it->set(matrix)->forEach([&result] (T &data) {
            result.push_back(data);
        });
        delete matrix;
        return result;
    }
}

#endif
