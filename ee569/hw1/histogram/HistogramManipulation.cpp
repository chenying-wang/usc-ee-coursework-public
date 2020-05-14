/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>
#include <algorithm>

#include "HistogramManipulation.h"
#include "common/Image.h"

Image *HistogramManipulation::manipulateByTransferFunction(const Image *oriImage, const char *transferFunctionFOutputFilename) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr) {
        return nullptr;
    }

    std::cout << "Manipulate by Transfer Function Start\n";
    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = oriImage->getChannel();
    const auto uiDepth = oriImage->getDepth();
    auto pcProcessedImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    pcProcessedImage->create();
    auto pppuhOriImage = oriImage->getImage();
    auto pppuhProcessedImage = pcProcessedImage->getImage();
    const auto uiPixels = uiWidth * uiHeight;

    auto ppuiCdf = this->cImageStat.calcCdf(oriImage);
    auto ppuiTransferFunction = new unsigned int*[MAX_PIXEL_VALUE + 1];
    for (auto i = 0u; i <= MAX_PIXEL_VALUE; ++i) {
        ppuiTransferFunction[i] = new unsigned int[uiChannel];
        for (auto j = 0u; j < uiChannel; ++j) {
            ppuiTransferFunction[i][j] = 1u * MAX_PIXEL_VALUE * ppuiCdf[i][j] / uiPixels;
        }
    }

    for (auto i = 0u; i < uiChannel; ++i) {
        for (auto j = 0u; j < uiHeight; ++j) {
            for (auto k = 0u; k < uiWidth; ++k) {
                pppuhProcessedImage[j][k][i] = ppuiTransferFunction[pppuhOriImage[j][k][i]][i];
            }
        }
    }
    std::cout << "Manipulate by Transfer Function Done\n";

    if (transferFunctionFOutputFilename == nullptr) {
        return pcProcessedImage;
    }

    this->cImageStat.writeCsv(ppuiTransferFunction, MAX_PIXEL_VALUE + 1, uiChannel, transferFunctionFOutputFilename);
    return pcProcessedImage;
}

Image *HistogramManipulation::manipulateByBucketFilling(const Image *oriImage) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr) {
        return nullptr;
    }

    std::cout << "Manipulate by Bucket Filling Start\n";
    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = oriImage->getChannel();
    const auto uiDepth = oriImage->getDepth();
    auto pcProcessedImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    pcProcessedImage->create();
    const auto uiDepthInBits = uiDepth << 3;

    auto ppuiCdf = this->cImageStat.calcCdf(oriImage);
    for (auto i = 0u; i < uiChannel; ++i) {
        pcProcessedImage = this->binarySplitImage(pcProcessedImage, oriImage, ppuiCdf, i, uiDepthInBits, uiDepthInBits - 1, 0x0);
    }
    std::cout << "Manipulate by Bucket Filling Done\n";
    return pcProcessedImage;
}

// call recursively
Image *HistogramManipulation::binarySplitImage(Image *processedImage, const Image *oriImage, unsigned int **cdf,
                                           const unsigned int channel, const unsigned int depthInBits,
                                           const unsigned int bit, const PIXEL_TYPE header) const {
    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiPixels = uiWidth * uiHeight;

    auto pppuhOriImage = oriImage->getImage();
    auto pppuhProcessedImage = processedImage->getImage();

    PIXEL_TYPE threshold;
    unsigned int uiNumOfPixelToSetZeroAtThreshold;
    unsigned int uiNumOfPixelAtThreshold;
    const unsigned int uiNumOfPixelToSetZero = uiPixels >> (depthInBits - bit);
    const unsigned int uiOffset = (header >> (bit + 1)) * (uiNumOfPixelToSetZero << 1);

    for (auto i = 0u; i <= MAX_PIXEL_VALUE; ++i) {
        if (cdf[i][channel] > uiOffset + uiNumOfPixelToSetZero) {
            threshold = i;

            if (i) {
                uiNumOfPixelToSetZeroAtThreshold = std::min(uiNumOfPixelToSetZero,
                    uiOffset + uiNumOfPixelToSetZero - cdf[i - 1][channel]);
                uiNumOfPixelAtThreshold = std::min(uiNumOfPixelToSetZero << 1,
                    cdf[i][channel] - cdf[i - 1][channel]);
            } else {
                uiNumOfPixelToSetZeroAtThreshold = std::min(uiNumOfPixelToSetZero,
                    uiOffset + uiNumOfPixelToSetZero);
                uiNumOfPixelAtThreshold = std::min(uiNumOfPixelToSetZero << 1,
                    cdf[i][channel]);
            }
            break;
        }
    }

    // 0 at bit, 1 at other bits
    const PIXEL_TYPE zeroAtBit = MAX_PIXEL_VALUE - (1 << bit);
    // 1 at bit, 0 at other bits
    const PIXEL_TYPE oneAtBit = 1 << bit;
    // pixel before bit which is not header will be ignored
    const PIXEL_TYPE mask = MAX_PIXEL_VALUE - ((1 << (bit + 1)) - 1);
    auto ppuiPixelsAtThreshold = new unsigned int*[uiNumOfPixelAtThreshold];
    auto uiNumOfValidPixelAtThreshold = 0u;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            if ((pppuhProcessedImage[i][j][channel] & mask) != header) {
                continue;
            }

            auto oriPixelValue = pppuhOriImage[i][j][channel];
            if (oriPixelValue < threshold) {
                pppuhProcessedImage[i][j][channel] &= zeroAtBit;
            } else if (oriPixelValue > threshold) {
                pppuhProcessedImage[i][j][channel] |= oneAtBit;
            } else {
                ppuiPixelsAtThreshold[uiNumOfValidPixelAtThreshold++] = new unsigned int[2]{i, j};
            }
        }
    }

    std::random_shuffle(ppuiPixelsAtThreshold, uiNumOfValidPixelAtThreshold + ppuiPixelsAtThreshold);
    for (auto i = 0u; i < uiNumOfValidPixelAtThreshold; ++i) {
        if (i < uiNumOfPixelToSetZeroAtThreshold) {
            pppuhProcessedImage[ppuiPixelsAtThreshold[i][0]][ppuiPixelsAtThreshold[i][1]][channel] &= zeroAtBit;
        } else {
            pppuhProcessedImage[ppuiPixelsAtThreshold[i][0]][ppuiPixelsAtThreshold[i][1]][channel] |= oneAtBit;
        }
    }

    if (!bit) {
        return processedImage;
    }

    processedImage = this->binarySplitImage(processedImage, oriImage, cdf, channel, depthInBits, bit - 1, header);
    processedImage = this->binarySplitImage(processedImage, oriImage, cdf, channel, depthInBits, bit - 1, header | oneAtBit);
    return processedImage;
}
