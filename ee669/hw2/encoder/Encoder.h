#ifndef __EE669_ENCODER
#define __EE669_ENCODER

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>

#include "Code.h"
#include "common/ByteStreamProcess.h"

template<typename T>
class Encoder : public ByteStreamProcess<T> {
private:
    std::vector<T> process0(std::vector<T> &input) override;

protected:
    std::unordered_map<T, Code<T>> codebook;
    double entropy;

    Encoder();
    virtual Code<T> encodeSymbol(T symbol, const uint64_t index, const uint64_t numSymbols);
    Encoder<T> *calcEntropy(const std::unordered_map<T, uint64_t> &stat);

public:
    virtual ~Encoder();

    virtual Encoder<T> *build(const std::unordered_map<T, uint64_t> &stat) = 0;
    template<typename Iterator>
    std::pair<std::vector<std::byte>, uint64_t> encode(Iterator first, Iterator last);

    const std::unordered_map<T, Code<T>> &getCodebook() const {
        return this->codebook;
    }

    double getEntropy() const {
        return this->entropy;
    }
};

template<typename T>
Encoder<T>::Encoder() : ByteStreamProcess<T>() {
    this->entropy = -1.0;
}

template<typename T>
Encoder<T>::~Encoder() {}

template<typename T>
template<typename Iterator>
std::pair<std::vector<std::byte>, uint64_t>
Encoder<T>::encode(Iterator first, Iterator last) {
    std::vector<std::byte> codes;
    if (!this->codebook.empty()) {
        uint64_t bitLen = 0UL;
        for (Iterator it = first; it != last; ++it) {
            const auto symbol = *it;
            if (!this->codebook.count(symbol)) {
                throw std::runtime_error("Unknown symbol found");
            }
            bitLen += this->codebook.at(symbol).codeLen;
        }
        const auto len = (bitLen + CHAR_BIT - 1) / CHAR_BIT;
        codes.reserve(len);
    }

    uint64_t bitLen = 0UL;
    uint64_t index = 0UL;
    const uint64_t numSymbols = std::distance(first, last);
    uint8_t alignment = 0U;
    for (Iterator it = first; it != last; ++it) {
        const auto &c = this->encodeSymbol(*it, index, numSymbols);
        auto code = c.code;
        auto offset = c.codeLen;
        bitLen += offset;
        while (offset > 0) {
            if (!alignment) {
                codes.push_back(std::byte(0U));
                alignment = CHAR_BIT;
            }
            if (alignment < offset) {
                offset -= alignment;
                codes.back() |= std::byte((code >> offset).to_ulong());
                code &= (1 << offset) - 1;
                alignment = 0U;
                continue;
            }
            alignment -= offset;
            codes.back() |= std::byte((code << alignment).to_ulong());
            offset = 0;
        }
        ++index;
    }
    return std::make_pair(codes, bitLen);
}

template<typename T>
Code<T>
Encoder<T>::encodeSymbol(T symbol, const uint64_t, const uint64_t) {
    return this->codebook.at(symbol);
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

template<typename T>
std::vector<T>
Encoder<T>::process0(std::vector<T> &input) {
    const auto &[result, _] = this->encode(input.cbegin(), input.cend());
    return result;
}

#endif
