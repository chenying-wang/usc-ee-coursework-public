/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <limits>

#include "Filter.h"
#include "Common.h"

Filter::Filter(
    const float **weight,
    const unsigned int width,
    const unsigned int height) :
    m_ppfWeight(weight),
    m_uiWidth(width),
    m_uiHeight(height),
    m_fWeightSum(this->sum(weight, width, height)) {}

Filter::Filter(
        const float **weight,
        const unsigned int width,
        const unsigned int height,
        const float weightSum) :
    m_ppfWeight(weight),
    m_uiWidth(width),
    m_uiHeight(height),
    m_fWeightSum(weightSum) {}


Filter::~Filter() {
    this->destroy();
}

Filter *Filter::destroy() {
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        delete[] this->m_ppfWeight[i];
    }
    delete[] this->m_ppfWeight;
    return this;
}

PIXEL_TYPE Filter::filterImage(const Image *image, const int x, const int y, const unsigned int channel) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return DEFAULT_PIXEL_VALUE;
    }

    const auto uiWidth = this->m_uiWidth;
    const auto uiHeight = this->m_uiHeight;
    const int xOffset = x - (uiHeight - 1) / 2, yOffset = y - (uiWidth - 1) / 2;

    float weightedSum = 0u;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            weightedSum += this->m_ppfWeight[i][j] * image->getPixel(xOffset + i, yOffset + j)[channel];
        }
    }
    return this->norm(weightedSum);
}

float Filter::filterImageWithoutNorm(const Image *image, const int x, const int y, const unsigned int channel) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return DEFAULT_PIXEL_VALUE;
    }

    const auto uiWidth = this->m_uiWidth;
    const auto uiHeight = this->m_uiHeight;
    const int xOffset = x - (uiHeight - 1) / 2, yOffset = y - (uiWidth - 1) / 2;

    float weightedSum = 0u;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            weightedSum += this->m_ppfWeight[i][j] * image->getPixel(xOffset + i, yOffset + j)[channel];
        }
    }
    return weightedSum / this->m_fWeightSum;
}

Image *Filter::filterImage(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    const auto uiDepth = image->getDepth();
    auto pcFilteredImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, image->getPaddingType()))->create();
    auto pppuhFilterImage = pcFilteredImage->getImage();

    for (auto i = 0u; i < uiChannel; ++i) {
        for (auto j = 0u; j < uiHeight; ++j) {
            for (auto k = 0u; k < uiWidth; ++k) {
                pppuhFilterImage[j][k][i] = this->filterImage(image, j, k, i);
            }
        }
    }
    return pcFilteredImage;
}

float Filter::sum(const float **weight,
                  const unsigned int width,
                  const unsigned int height) const {
    if (weight == nullptr) {
        return 1.0f;
    }

    auto sum = .0f;
    for (auto i = 0u; i < height; ++i) {
        for (auto j = 0u; j < width; ++j) {
            sum += std::abs(weight[i][j]);
        }
    }
    if (sum <= std::numeric_limits<float>::epsilon()) {
        return 1.0f;
    }
    return sum;
}

PIXEL_TYPE Filter::norm(const float weightedSum, const float totalWeight) {
    float _weightedSum = 0, _absWeightSum = 0;
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
        return MAX_PIXEL_VALUE;
    }
    return (_weightedSum + _absWeightSum / 2) / _absWeightSum;
}

PIXEL_TYPE Filter::norm(const float weightedSum) const {
    return Filter::norm(weightedSum, this->m_fWeightSum);
}
