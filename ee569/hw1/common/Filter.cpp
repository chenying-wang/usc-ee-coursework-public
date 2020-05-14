/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>

#include "Filter.h"
#include "Common.h"

Filter::~Filter() {
    this->destroy();
}

Filter *Filter::destroy() {
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        delete[] this->m_ppiWeight[i];
    }
    return this;
}

PIXEL_TYPE Filter::filterImage(const Image *image, const int x, const int y, const unsigned int channel) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return DEFAULT_PIXEL_VALUE;
    }

    const auto uiWidth = this->getWidth();
    const auto uiHeight = this->getHeight();
    const int xOffset = x - (uiHeight - 1) / 2, yOffset = y - (uiWidth - 1) / 2;

    float weightedSum = 0u;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            weightedSum += this->m_ppiWeight[i][j] * image->getPixel(xOffset + i, yOffset + j)[channel];
        }
    }
    return this->norm(weightedSum);
}

float Filter::sum(float **weight,
                 const unsigned int width,
                 const unsigned int height) const {
    if (weight == nullptr) {
        return 0;
    }

    float sum = 0;
    for (auto i = 0u; i < height; ++i) {
        for (auto j = 0u; j < width; ++j) {
            sum += weight[i][j];
        }
    }
    return sum;
}

PIXEL_TYPE Filter::norm(const float weightedSum, const float totalWeight) {
    int _weightedSum = 0, _absWeightSum = 0;
    if (totalWeight < 0) {
        _weightedSum = -weightedSum;
        _absWeightSum = -totalWeight;
    } else {
        _weightedSum = weightedSum;
        _absWeightSum = totalWeight;
    }
    if (_weightedSum <= 0) {
        return 0;
    } else if (_weightedSum >= _absWeightSum * MAX_PIXEL_VALUE) {
        return UINT8_MAX;
    }
    return (_weightedSum + _absWeightSum / 2) / _absWeightSum;
}

PIXEL_TYPE Filter::norm(const float weightedSum) const {
    return Filter::norm(weightedSum, this->m_iWeightSum);
}
