#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstddef>
#include <climits>

#include "Encoder.h"
#include "third_party/AdaptiveHuffman.h"

#ifndef __EE669_ADAPTIVE_HUFFMAN_ENCODER
#define __EE669_ADAPTIVE_HUFFMAN_ENCODER

template<typename T>
class AdaptiveHuffmanEncoder : public Encoder<T> {
public:
    AdaptiveHuffmanEncoder();
    virtual ~AdaptiveHuffmanEncoder();

    Encoder<T> *build(const std::unordered_map<T, uint64_t> &stat) override;
    std::pair<std::vector<std::byte>, uint64_t> encode(const std::vector<T> &input) const override;
};

template<typename T>
AdaptiveHuffmanEncoder<T>::AdaptiveHuffmanEncoder() {}

template<typename T>
AdaptiveHuffmanEncoder<T>::~AdaptiveHuffmanEncoder() {}

template<typename T>
Encoder<T> *
AdaptiveHuffmanEncoder<T>::build(const std::unordered_map<T, uint64_t> &stat) {
    return this->calcEntropy(stat);
}

template<typename T>
std::pair<std::vector<std::byte>, uint64_t>
AdaptiveHuffmanEncoder<T>::encode(const std::vector<T> &input) const {
    auto iss = std::istringstream(std::string(reinterpret_cast<const char *>(input.data()), input.size() * sizeof(T)));
    auto oss = std::ostringstream();
    std::istream &in = iss;
    std::ostream &out = oss;
    vitter::compress(&in, &out);
    const auto output = oss.str();
    const auto codes = std::vector<std::byte>(reinterpret_cast<const std::byte *>(output.data()),
            reinterpret_cast<const std::byte *>(output.data() + output.size()));
    const auto bitLen = CHAR_BIT * codes.size();
    return std::make_pair(codes, bitLen);
}

#endif
