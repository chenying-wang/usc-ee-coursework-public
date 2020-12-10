#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>

#include "Code.h"

#ifndef __EE669_ENCODER
#define __EE669_ENCODER

template<typename T>
class Encoder {
protected:
    std::unordered_map<T, Code<T>> codebook;
    double entropy;

    Encoder();
    Encoder<T> *calcEntropy(const std::unordered_map<T, uint64_t> &stat);

public:
    virtual ~Encoder();

    virtual Encoder<T> *build(const std::unordered_map<T, uint64_t> &stat) = 0;
    virtual std::pair<std::vector<std::byte>, uint64_t> encode(const std::vector<T> &input) const;
    template<typename Iterator>
    std::pair<std::vector<std::byte>, uint64_t> encode(Iterator first, Iterator last) const;

    const std::unordered_map<T, Code<T>> &getCodebook() const {
        return this->codebook;
    }

    double getEntropy() const {
        return this->entropy;
    }
};

template<typename T>
Encoder<T>::Encoder() {
    this->entropy = -1.0;
}

template<typename T>
Encoder<T>::~Encoder() {}

template<typename T>
std::pair<std::vector<std::byte>, uint64_t>
Encoder<T>::encode(const std::vector<T> &input) const {
    return this->encode(input.cbegin(), input.cend());
}

template<typename T>
template<typename Iterator>
std::pair<std::vector<std::byte>, uint64_t>
Encoder<T>::encode(Iterator first, Iterator last) const {
    uint64_t bitLen = 0UL;
    for (Iterator it = first; it != last; ++it) {
        const auto symbol = *it;
        if (!this->codebook.count(symbol)) {
            throw std::runtime_error("Unknown symbol found");
        }
        bitLen += this->codebook.at(symbol).codeLen;
    }
    const auto len = (bitLen + CHAR_BIT - 1) / CHAR_BIT;

    auto codes = std::vector<std::byte>(len, std::byte(0));
    uint64_t index = 0UL;
    uint8_t alignment = CHAR_BIT;
    for (Iterator it = first; it != last; ++it) {
        const auto &c = this->codebook.at(*it);
        auto code = c.code;
        auto offset = c.codeLen;
        while (offset > 0) {
            if (alignment < offset) {
                offset -= alignment;
                codes[index] |= std::byte((code >> offset).to_ulong());
                code &= (1 << offset) - 1;
                ++index;
                alignment = CHAR_BIT;
                continue;
            }
            alignment -= offset;
            codes[index] |= std::byte((code << alignment).to_ulong());
            if (!alignment) {
                ++index;
                alignment = CHAR_BIT;
            }
            offset = 0;
        }
    }
    return std::make_pair(codes, bitLen);
}

template<typename T>
Encoder<T> *
Encoder<T>::calcEntropy(const std::unordered_map<T, uint64_t> &stat) {
    if (stat.empty()) {
        this->entropy = .0;
        return this;
    }

    uint64_t numSymbols = 0UL;
    for (const auto &e : stat) {
        numSymbols += e.second;
    }
    double entropy = .0;
    for (const auto &e : stat) {
        double prob = (double) e.second / numSymbols;
        entropy -= prob * std::log2(prob);
    }
    this->entropy = entropy;
    return this;
}

#endif
