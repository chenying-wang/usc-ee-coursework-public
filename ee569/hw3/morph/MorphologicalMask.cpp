/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>

#include "MorphologicalMask.h"

MorphologicalMask::MorphologicalMask(const uint8_t **mask) :
    m_ppuhMask(mask) {}

MorphologicalMask::~MorphologicalMask() {}

bool MorphologicalMask::evaluate(const Image *image, const unsigned int x, const unsigned int y,
                                 const unsigned int channel) const {
    if (image == nullptr ||
        image->getImage() == nullptr) {
        return false;
    }

    if (x == 0 || x == image->getHeight() - 1) {
        return false;
    } else if (y == 0 || y == image->getWidth() - 1) {
        return false;
    }

    const auto pppuhImage = image->getImage();
    bool bHasTwo = false, bHitAnyTwo = false;
    for (auto i = 0u; i < 3u; ++i) {
        for (auto j = 0u; j < 3u; ++j) {
            if (this->m_ppuhMask[i][j] == 0u && pppuhImage[x + i - 1][y + j - 1][channel]) {
                return false;
            } else if (this->m_ppuhMask[i][j] == 1u && !pppuhImage[x + i - 1][y + j - 1][channel]) {
                return false;
            } else if (this->m_ppuhMask[i][j] == 2u && !bHitAnyTwo) {
                bHasTwo = true;
                bHitAnyTwo = pppuhImage[x + i - 1][y + j - 1][channel] > 0;
            }
        }
    }
    return bHasTwo == bHitAnyTwo;
}
