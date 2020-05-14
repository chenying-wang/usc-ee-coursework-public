/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>
#include <cmath>
#include <cstring>

#include "ImageWarp.h"

ImageWarp::ImageWarp() {}

ImageWarp::~ImageWarp() {}

Image *ImageWarp::warp(const Image *oriImage) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = oriImage->getChannel();
    const auto uiDepth = oriImage->getDepth();
    const auto pppuhOriImage = oriImage->getImage();

    return oriImage->concurrentMap([=](unsigned int i, unsigned int j) {
        const auto fFactor = this->calculateFactor(i, j, uiWidth, uiHeight);
        const auto fMappedX = uiHeight / 2.f + (i - uiHeight / 2.f) * fFactor;
        const auto fMappedY = uiWidth / 2.f + (j - uiWidth / 2.f) * fFactor;

        const unsigned int uiMappedXFloor = std::floor(fMappedX);
        const unsigned int uiMappedYFloor = std::floor(fMappedY);
        const auto fMappedXOffset = fMappedX - uiMappedXFloor;
        const auto fMappedYOffset = fMappedY - uiMappedYFloor;

        const auto puhResult = new PIXEL_TYPE[uiChannel];
        if (uiMappedXFloor >= uiHeight || uiMappedYFloor >= uiWidth) {
            std::memset(puhResult, DEFAULT_PIXEL_VALUE, uiChannel * uiDepth);
            return puhResult;
        }

        for (auto k = 0u; k < uiChannel; ++k) {
            puhResult[k] =
                (1 - fMappedXOffset) * (1 - fMappedYOffset) * pppuhOriImage[uiMappedXFloor][uiMappedYFloor][k] +
                fMappedXOffset * (1 - fMappedYOffset) * pppuhOriImage[std::min(uiMappedXFloor + 1, uiHeight - 1)][uiMappedYFloor][k] +
                (1 - fMappedXOffset) * fMappedYOffset * pppuhOriImage[uiMappedXFloor][std::min(uiMappedYFloor + 1, uiWidth - 1)][k] +
                fMappedXOffset * fMappedYOffset * pppuhOriImage[std::min(uiMappedXFloor + 1, uiHeight - 1)][std::min(uiMappedYFloor + 1, uiWidth - 1)][k];
        }
        return puhResult;
    });
}

Image *ImageWarp::reverseWarp(const Image *warppedImage) const {
    if (warppedImage == nullptr || warppedImage->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = warppedImage->getWidth();
    const auto uiHeight = warppedImage->getHeight();
    const auto uiChannel = warppedImage->getChannel();
    const auto uiDepth = warppedImage->getDepth();
    const auto pppuhWarppedImage = warppedImage->getImage();

    return warppedImage->concurrentMap([=](unsigned int i, unsigned int j) {
        const auto fFactor = this->calculateFactor(i, j, uiWidth, uiHeight);
        const auto fMappedX = uiHeight / 2.f + (i - uiHeight / 2.f) / fFactor;
        const auto fMappedY = uiWidth / 2.f + (j - uiWidth / 2.f) / fFactor;

        const unsigned int uiMappedXFloor = std::floor(fMappedX);
        const unsigned int uiMappedYFloor = std::floor(fMappedY);
        const auto fMappedXOffset = fMappedX - uiMappedXFloor;
        const auto fMappedYOffset = fMappedY - uiMappedYFloor;

        const auto puhResult = new PIXEL_TYPE[uiChannel];
        if (uiMappedXFloor >= uiHeight || uiMappedYFloor >= uiWidth) {
            std::memset(puhResult, DEFAULT_PIXEL_VALUE, uiChannel * uiDepth);
            return puhResult;
        }

        for (auto k = 0u; k < uiChannel; ++k) {
            puhResult[k] =
                (1 - fMappedXOffset) * (1 - fMappedYOffset) * pppuhWarppedImage[uiMappedXFloor][uiMappedYFloor][k] +
                fMappedXOffset * (1 - fMappedYOffset) * pppuhWarppedImage[std::min(uiMappedXFloor + 1, uiHeight - 1)][uiMappedYFloor][k] +
                (1 - fMappedXOffset) * fMappedYOffset * pppuhWarppedImage[uiMappedXFloor][std::min(uiMappedYFloor + 1, uiWidth - 1)][k] +
                fMappedXOffset * fMappedYOffset * pppuhWarppedImage[std::min(uiMappedXFloor + 1, uiHeight - 1)][std::min(uiMappedYFloor + 1, uiWidth - 1)][k];
        }
        return puhResult;
    });
}

float ImageWarp::calculateFactor(const unsigned int i, const unsigned int j,
                                  const unsigned int width, const unsigned int height) const {
    const auto x = std::abs(i - height / 2.f);
    const auto y = std::abs(j - width / 2.f);
    if (x < std::numeric_limits<float>::epsilon() && y < std::numeric_limits<float>::epsilon()) {
        return 1.;
    }
    const auto distanceSquare = x * x + y * y;
    return x * width > y * height ?
        std::sqrt(distanceSquare) / x : std::sqrt(distanceSquare) / y;
}
