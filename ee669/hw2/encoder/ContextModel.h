#ifndef __EE669_CONTEXT_MODEL
#define __EE669_CONTEXT_MODEL

#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>

class ContextModel {
protected:
    const uint8_t numContextBits;

public:
    ContextModel(const uint8_t numContextBits);
    virtual ~ContextModel();

    virtual uint64_t operator+=(const bool bit) = 0;

    uint8_t getNumContextBits() const {
        return this->numContextBits;
    }
};

ContextModel::ContextModel(const uint8_t numContextBits) :
    numContextBits(std::min(numContextBits, static_cast<uint8_t>(16U))) {}

ContextModel::~ContextModel() {}

class BitsContextModel : public ContextModel {
    const uint64_t mask;
    uint64_t context;

public:
    BitsContextModel(const uint8_t numContextBits);
    virtual ~BitsContextModel();

    uint64_t operator+=(const bool bit) override;
};

BitsContextModel::BitsContextModel(const uint8_t numContextBits) :
    ContextModel(numContextBits), mask((1U << numContextBits) - 1) {
    this->context = 0U;
}

BitsContextModel::~BitsContextModel() {}

uint64_t
BitsContextModel::operator+=(const bool bit) {
    if (!this->numContextBits) {
        return 0UL;
    }
    const auto ctx = this->context;
    this->context <<= 1;
    this->context &= this->mask;
    this->context |= bit;
    return ctx;
}

#endif
