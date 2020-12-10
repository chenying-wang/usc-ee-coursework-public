#ifndef __EE669_QM_ENCODER
#define __EE669_QM_ENCODER

#include <iostream>
#include <unordered_map>
#include <cstdio>

#include "Code.h"
#include "ContextModel.h"
#include "Encoder.h"
#include "third_party/qmcoder.h"

template<typename T>
class QMEncoder : public Encoder<T> {
private:
    ContextModel *contextModel;
    QM *qm;
    char *buffer;
    size_t bufferSize;
    size_t offset;
    FILE *mem_stream;

    Code<T> encodeSymbol(T symbol, const uint64_t index, const uint64_t numSymbols) override;

public:
    QMEncoder(ContextModel *contextModel);
    virtual ~QMEncoder();

    Encoder<T> *build(const std::unordered_map<T, uint64_t> &stat) override;
};

template<typename T>
QMEncoder<T>::QMEncoder(ContextModel *contextModel) : contextModel(contextModel) {
    this->mem_stream = open_memstream(&this->buffer, &this->bufferSize);
    this->qm = new QM(this->mem_stream, 1 << contextModel->getNumContextBits());
    this->qm->StartQM("encode");
    this->offset = 0UL;
}

template<typename T>
QMEncoder<T>::~QMEncoder() {
    if (this->contextModel != nullptr) {
        delete this->contextModel;
    }
    if (this->qm != nullptr) {
        delete this->qm;
    }
    if (this->mem_stream != nullptr) {
        fclose(this->mem_stream);
    }
    if (this->buffer != nullptr) {
        delete[] this->buffer;
    }
}

template<typename T>
Encoder<T> *
QMEncoder<T>::build(const std::unordered_map<T, uint64_t> &stat) {
    return this->calcEntropy(stat);
}

template<typename T>
Code<T>
QMEncoder<T>::encodeSymbol(T symbol, const uint64_t index, const uint64_t numSymbols) {
    const static auto numBitsT = CHAR_BIT * sizeof(T);
    for (auto i = 0u; i < numBitsT; ++i) {
        bool bit = (uint64_t) symbol & (1 << (numBitsT - 1 - i));
        auto ctx = *(this->contextModel) += bit;
        this->qm->encode(bit, ctx);
    }
    if (index == numSymbols - 1) {
        this->qm->Flush();
    }
    fflush(this->mem_stream);
    Code<T> code(1UL, symbol);
    if (this->offset >= this->bufferSize) {
        return code;
    }
    code.codeLen = CHAR_BIT * (this->bufferSize - this->offset);
    while (this->offset < this->bufferSize) {
        code.code <<= CHAR_BIT;
        code.code |= this->buffer[this->offset];
        ++this->offset;
    }
    return code;
}

#endif
