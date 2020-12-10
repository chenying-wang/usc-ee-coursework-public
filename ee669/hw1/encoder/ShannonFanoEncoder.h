#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <bitset>
#include <cstddef>

#include "Code.h"
#include "Encoder.h"
#include "third_party/ShannonFano.h"

#ifndef __EE669_SHANNON_FANO_ENCODER
#define __EE669_SHANNON_FANO_ENCODER

template<typename T>
class ShannonFanoEncoder : public Encoder<T> {
private:
    Code<T> &adaptResult(Code<T> &to, third_party::SymbolCode &from) const;

public:
    ShannonFanoEncoder();
    virtual ~ShannonFanoEncoder();

    Encoder<T> *build(const std::unordered_map<T, uint64_t> &stat) override;
};

template<typename T>
ShannonFanoEncoder<T>::ShannonFanoEncoder() {}

template<typename T>
ShannonFanoEncoder<T>::~ShannonFanoEncoder() {}

template<typename T>
Encoder<T> *
ShannonFanoEncoder<T>::build(const std::unordered_map<T, uint64_t> &stat) {
    this->codebook.clear();
    if (stat.empty()) {
        return this;
    } else if (stat.size() == 1) {
        auto c = Code<T>(stat.cbegin()->second, stat.cbegin()->first);
        c.codeLen = 1;
        this->codebook.insert(std::make_pair(stat.cbegin()->first, c));
        return this;
    }

    auto symbols = std::vector<T>();
    for (const auto &e : stat) {
        symbols.push_back(e.first);
    }
    std::sort(symbols.begin(), symbols.end(), [&stat](T a, T b) {
        return std::greater<int>()(stat.at(a), stat.at(b));
    });
    auto symbolCodes = std::vector<third_party::SymbolCode>();
    for (const auto &symbol : symbols) {
        symbolCodes.push_back(third_party::SymbolCode(0, stat.at(symbol)));
    }
    third_party::shannonFano(symbolCodes, 0, symbolCodes.size() - 1);

    for (auto i = 0UL; i < symbols.size(); ++i) {
        auto c = Code<T>(stat.at(symbols[i]), symbols[i]);
        this->codebook.insert(std::make_pair(symbols[i], this->adaptResult(c, symbolCodes[i])));
    }
    return this->calcEntropy(stat);
}

template<typename T>
Code<T> &
ShannonFanoEncoder<T>::adaptResult(Code<T> &to, third_party::SymbolCode &from) const {
    const auto &code = from.getCode();
    to.codeLen = code.length();
    for (auto i = 0UL; i < to.codeLen; ++i) {
        if (code.at(to.codeLen - 1 - i) == '1') {
            to.code.set(i);
        }
    }
    return to;
}

#endif
