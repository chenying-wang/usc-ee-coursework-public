/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <iostream>
#include <cstring>

#include "ImageConverter.h"

ImageConverter::ImageConverter() {}

ImageConverter::~ImageConverter() {}

Image *ImageConverter::normalize(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    const auto uiDepth = image->getDepth();
    auto pcNormalizedImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, image->getPaddingType()))->create();

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
    if (uiChannel != NUM_OF_CHANNELS_RGB) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiDepth = image->getDepth();
    auto pcResultImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, image->getPaddingType()))->create();

    auto pppuhOriImage = image->getImage();
    auto pppuhResultImage = pcResultImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            for (auto k = 0u; k < uiChannel; ++k) {
                pppuhResultImage[i][j][k] = MAX_PIXEL_VALUE - pppuhOriImage[i][j][k];
            }
        }
    }
    return pcResultImage;
}

Image *ImageConverter::contertRGBtoY(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    if (uiChannel != NUM_OF_CHANNELS_RGB) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiDepth = image->getDepth();
    auto pcResultImage = (new Image(uiWidth, uiHeight, NUM_OF_CHANNELS_GRAY, uiDepth, image->getPaddingType()))->create();

    auto pppuhOriImage = image->getImage();
    auto pppuhResultImage = pcResultImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto puhOriPixel = pppuhOriImage[i][j];
            auto puhResultPixel = pppuhResultImage[i][j];
            puhResultPixel[CHANNEL_GRAY] = 0.5f + 0.2989f * puhOriPixel[CHANNEL_R] +
                0.5870f * puhOriPixel[CHANNEL_G] + 0.1140f * puhOriPixel[CHANNEL_B];
        }
    }
    return pcResultImage;
}

Image *ImageConverter::contertYtoRGB(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    if (uiChannel != NUM_OF_CHANNELS_GRAY) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiDepth = image->getDepth();
    auto pcResultImage = (new Image(uiWidth, uiHeight, NUM_OF_CHANNELS_RGB, uiDepth, image->getPaddingType()))->create();

    auto pppuhOriImage = image->getImage();
    auto pppuhResultImage = pcResultImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto puhOriPixel = pppuhOriImage[i][j];
            auto puhResultPixel = pppuhResultImage[i][j];
            puhResultPixel[CHANNEL_R] = puhOriPixel[CHANNEL_GRAY];
            puhResultPixel[CHANNEL_G] = puhOriPixel[CHANNEL_GRAY];
            puhResultPixel[CHANNEL_B] = puhOriPixel[CHANNEL_GRAY];
        }
    }
    return pcResultImage;
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
