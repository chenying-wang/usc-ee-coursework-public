/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <cstring>

#include "ImageConverter.h"

ImageConverter::ImageConverter() {}

ImageConverter::~ImageConverter() {}

Image *ImageConverter::duplicate(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    auto uiChannel = image->getChannel();
    auto uiDepth = image->getDepth();
    auto pppuhOriImage = image->getImage();
    return image->concurrentMap([uiChannel, uiDepth, pppuhOriImage](unsigned int i, unsigned int j) {
        auto puhResult = new PIXEL_TYPE[uiChannel];
        std::memcpy(puhResult, pppuhOriImage[i][j], uiChannel * uiDepth);
        return puhResult;
    }, 4u);
}

Image *ImageConverter::normalize(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    auto pcNormalizedImage = image->dumpStructure()->create();

    auto pppuhOriImage = image->getImage();
    auto pppuhNormalizedImage = pcNormalizedImage->getImage();
    for (auto i = 0u; i < uiChannel; ++i) {
        PIXEL_TYPE min= MAX_PIXEL_VALUE, max = 0;
        for (auto j = 0u; j < uiHeight; ++j) {
            for (auto k = 0u; k < uiWidth; ++k) {
                min = std::min(pppuhOriImage[j][k][i], min);
                max = std::max(pppuhOriImage[j][k][i], max);
            }
        }
        PIXEL_TYPE diff = max - min;
        for (auto j = 0u; j < uiHeight; ++j) {
            for (auto k = 0u; k < uiWidth; ++k) {
                pppuhNormalizedImage[j][k][i] = ((unsigned int) (pppuhOriImage[j][k][i] - min) * MAX_PIXEL_VALUE + diff / 2) / diff;
            }
        }
    }
    return pcNormalizedImage;
}

Image *ImageConverter::invert(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    const auto pppuhOriImage = image->getImage();
    return image->concurrentMap([uiChannel, pppuhOriImage](unsigned int i, unsigned int j) {
        auto puhResult = new PIXEL_TYPE[uiChannel];
        for (auto k = 0u; k < uiChannel; ++k) {
            puhResult[k] = MAX_PIXEL_VALUE - pppuhOriImage[i][j][k];
        }
        return puhResult;
    }, 4u);
}

Image *ImageConverter::binarize(const Image *image, const PIXEL_TYPE threshold) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    const auto pppuhOriImage = image->getImage();
    return image->concurrentMap([=](unsigned int i, unsigned int j) {
        auto puhResult = new PIXEL_TYPE[uiChannel];
        for (auto k = 0u; k < uiChannel; ++k) {
            puhResult[k] = pppuhOriImage[i][j][k] > threshold ? MAX_PIXEL_VALUE : 0;
        }
        return puhResult;
    }, 4u);
}

Image *ImageConverter::contertRGBtoY(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    if (uiChannel != NUM_OF_CHANNELS_RGB) {
        return nullptr;
    }
    auto pppuhOriImage = image->getImage();
    return image->concurrentMap([pppuhOriImage](unsigned int i, unsigned int j) {
        auto puhOriPixel = pppuhOriImage[i][j];
        auto puhResultPixel = new PIXEL_TYPE[NUM_OF_CHANNELS_GRAY];
        puhResultPixel[CHANNEL_GRAY] = 0.5f + 0.2989f * puhOriPixel[CHANNEL_R] +
            0.5870f * puhOriPixel[CHANNEL_G] + 0.1140f * puhOriPixel[CHANNEL_B];
        return puhResultPixel;
    }, 4u);
}

Image *ImageConverter::contertYtoRGB(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    if (uiChannel != NUM_OF_CHANNELS_GRAY) {
        return nullptr;
    }

    auto pppuhOriImage = image->getImage();
    return image->concurrentMap([pppuhOriImage](unsigned int i, unsigned int j) {
        auto puhOriPixel = pppuhOriImage[i][j];
        auto puhResultPixel = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB];
        puhResultPixel[CHANNEL_R] = puhOriPixel[CHANNEL_GRAY];
        puhResultPixel[CHANNEL_G] = puhOriPixel[CHANNEL_GRAY];
        puhResultPixel[CHANNEL_B] = puhOriPixel[CHANNEL_GRAY];
        return puhResultPixel;
    }, 4u);
}

Image *ImageConverter::contertRGBtoCMY(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    return this->invert(image);
}

Image *ImageConverter::contertCMYtoRGB(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    return this->invert(image);
}
