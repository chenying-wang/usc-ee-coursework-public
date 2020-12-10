#ifndef __EE669_HISTOGRAM_PROCESS
#define __EE669_HISTOGRAM_PROCESS

#include <iostream>
#include <vector>

#include "common/StreamProcess.h"

namespace ee669 {
    template<typename T>
    class HistogramProcess : public StreamProcess<T> {
    private:
        static_assert(std::is_integral<T>::value, "Template type T must be integral");

        std::vector<T> process0(std::vector<T> &input) override;

    public:
        HistogramProcess();
        virtual ~HistogramProcess();
    };
}

#endif
