#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstddef>

#include "HistogramProcess.h"
#include "common/StreamProcess.h"
#include "common/Utils.h"

using ee669::HistogramProcess;

template<>
HistogramProcess<uint8_t>::HistogramProcess() : ee669::StreamProcess<uint8_t>() {}

template<>
HistogramProcess<uint8_t>::~HistogramProcess() {}

template<>
std::vector<uint8_t> HistogramProcess<uint8_t>::process0(std::vector<uint8_t> &input) {
    const auto stat = ee669::Utils::stat(input.cbegin(), input.cend());
    std::ostringstream oss;
    for (const auto &[k, v] : stat) {
        oss << static_cast<uint64_t>(k)  << ',' << v << '\n';
    }
    const auto output = oss.str();
    std::vector<uint8_t> byteStream;
    byteStream.insert(byteStream.end(), reinterpret_cast<const uint8_t *>(output.data()),
        reinterpret_cast<const uint8_t *>(output.data() + output.size()));
    return byteStream;
}
