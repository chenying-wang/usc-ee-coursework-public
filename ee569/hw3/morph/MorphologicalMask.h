/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __MORPHOLOGICAL_MASK
#define __MORPHOLOGICAL_MASK

#include <iostream>

#include "common/Image.h"

class MorphologicalMask {
private:
    const uint8_t **m_ppuhMask;

public:
    MorphologicalMask(const uint8_t **mask);
    virtual ~MorphologicalMask();

    bool evaluate(const Image *image, const unsigned int x, const unsigned int y,
                  const unsigned int channel) const;

    const uint8_t **getMask() const {
        return this->m_ppuhMask;
    }
};

#endif
