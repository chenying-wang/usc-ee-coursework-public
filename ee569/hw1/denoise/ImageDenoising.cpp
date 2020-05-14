/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>
#include <cmath>

#include "ImageDenoising.h"
#include "common/Filter.h"

ImageDenoising::ImageDenoising() {}

ImageDenoising::~ImageDenoising() {}

Image *ImageDenoising::uniformFilterImage(const Image *oriImage, const unsigned int filterWidth,
                                          const unsigned int filterHeight) const {
    std::cout << "Uniform Denoising Start\n";
    auto weight = new float*[filterHeight];
    for (auto i = 0u; i < filterHeight; ++i) {
        weight[i] = new float[filterWidth];
        for (auto j = 0u; j < filterWidth; ++j) {
            weight[i][j] = 1;
        }
    }
    auto pcUniformFilter = new Filter(weight, filterWidth, filterHeight);
    auto pcFilteredImage = this->filterImage(oriImage, pcUniformFilter, filterWidth, filterHeight);
    std::cout << "Uniform Denoising Done\n";
    return pcFilteredImage;
}

Image *ImageDenoising::gaussianFilterImage(const Image *oriImage, const unsigned int filterWidth,
                                           const unsigned int filterHeight, const float sigma) const {
    std::cout << "Gaussian Denoising Start\n";
    auto weight = new float*[filterHeight];
    const float fDoubleSigmaSquare = 2 * sigma * sigma;
    for (auto i = 0u; i < filterHeight; ++i) {
        weight[i] = new float[filterWidth];
        float dx = i - filterHeight / 2.0;
        for (auto j = 0u; j < filterWidth; ++j) {
            float dy = j - filterWidth / 2.0;
            weight[i][j] = exp( -(dx*dx + dy*dy) / fDoubleSigmaSquare) / sigma;
        }
    }
    auto pcGaussianFilter = new Filter(weight, filterWidth, filterHeight);
    auto pcFilteredImage = this->filterImage(oriImage, pcGaussianFilter, filterWidth, filterHeight);
    std::cout << "Gaussian Denoising Done\n";
    return pcFilteredImage;
}

Image *ImageDenoising::bilateralFilterImage(const Image *oriImage, const unsigned int filterWidth, const unsigned int filterHeight,
                                            const float sigmaS, const float sigmaC) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr || filterWidth == 0 || filterHeight == 0) {
        return nullptr;
    }

    std::cout << "Bilateral Denoising Start\n";
    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = oriImage->getChannel();
    const auto uiDepth = oriImage->getDepth();
    auto pcFilteredImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    auto pppuhFilterImage = pcFilteredImage->create()->getImage();

    const float fDoubleSigmaSSquare = 2 * sigmaS * sigmaS;
    const float fDoubleSigmaCSquare = 2 * sigmaC * sigmaC;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto centerPixel = oriImage->getPixel(i, j)[CHANNEL_GRAY];
            auto weight = new float*[filterHeight];
            for (auto k = 0u; k < filterHeight; ++k) {
                weight[k] = new float[filterWidth];
                int dx = k - (filterHeight - 1) / 2;
                for (auto l = 0u; l < filterWidth; ++l) {
                    int dy = l - (filterWidth - 1) / 2;
                    auto iDistanceSquare = dx * dx + dy * dy;

                    int iPixelDifference = 0 + centerPixel - oriImage->getPixel(i + dx, j + dy)[CHANNEL_GRAY];
                    auto iPixelDifferenceSquare = iPixelDifference * iPixelDifference;
                    weight[k][l] = exp(-iDistanceSquare / fDoubleSigmaSSquare - iPixelDifferenceSquare / fDoubleSigmaCSquare);
                }
            }
            auto filter = new Filter(weight, filterWidth, filterHeight);
            pppuhFilterImage[i][j][CHANNEL_GRAY] = filter->filterImage(oriImage, i, j, CHANNEL_GRAY);
            delete filter;
        }
    }

    std::cout << "Bilateral Denoising Done\n";
    return pcFilteredImage;
}

Image *ImageDenoising::nlmFilterImage(const Image *oriImage, const unsigned int filterWidth, const unsigned int filterHeight,
                                      const unsigned int neighborhoodWidth, const unsigned int neighborhoodHeight,
                                      const float h, const float a) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr || filterWidth == 0 || filterHeight == 0) {
        return nullptr;
    }

    std::cout << "Non-local Means Denoising Start\n";
    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = oriImage->getChannel();
    const auto uiDepth = oriImage->getDepth();
    auto pcFilteredImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    auto pppuhFilterImage = pcFilteredImage->create()->getImage();

    const float fHSquare = 2 * h * h;
    const float fDoubleASquare = 2 * a * a;
    const float fGaConstant = sqrt(M_1_PI / 2) / a;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto weight = new float*[filterHeight];
            for (auto k = 0u; k < filterHeight; ++k) {
                weight[k] = new float[filterWidth];
                int dk = k - (filterHeight - 1) / 2;
                for (auto l = 0u; l < filterWidth; ++l) {
                    float fGaussianWeightedEuclidianDistanceSquare = 0;
                    int dl = l - (filterWidth - 1) / 2;
                    for (auto u = 0u; u < neighborhoodHeight; ++u) {
                        int du = u - (neighborhoodHeight - 1) / 2;
                        for (auto v = 0u; v < neighborhoodWidth; ++v) {
                            int dv = v - (neighborhoodWidth - 1) / 2;
                            int iPixelDifference = oriImage->getPixel(i + du, j + dv)[CHANNEL_GRAY] -
                                oriImage->getPixel(i + dk + du, j + dl + dv)[CHANNEL_GRAY];
                            fGaussianWeightedEuclidianDistanceSquare += iPixelDifference * iPixelDifference *
                                fGaConstant * exp(-(du*du + dv*dv) / fDoubleASquare);
                        }
                    }
                    weight[k][l] = exp(-fGaussianWeightedEuclidianDistanceSquare / fHSquare);
                }
            }
            auto filter = new Filter(weight, filterWidth, filterHeight);
            pppuhFilterImage[i][j][CHANNEL_GRAY] = filter->filterImage(oriImage, i, j, CHANNEL_GRAY);
            delete filter;
        }
#ifdef __DEBUG
        std::cout << i << " row done\n";
#endif
    }

    std::cout << "Non-local Means Denoising Done\n";
    return pcFilteredImage;
}

Image *ImageDenoising::filterImage(const Image *oriImage, const Filter *filter,
                                   const unsigned int filterWidth, const unsigned int filterHeight) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr || filterWidth == 0 || filterHeight == 0) {
        return nullptr;
    }

    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = oriImage->getChannel();
    const auto uiDepth = oriImage->getDepth();
    auto pcFilteredImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    auto pppuhFilterImage = pcFilteredImage->create()->getImage();

    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            pppuhFilterImage[i][j][CHANNEL_GRAY] = filter->filterImage(oriImage, i, j, CHANNEL_GRAY);
        }
    }

    return pcFilteredImage;
}
