/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <iostream>
#include <cmath>

#include "opencv2/imgproc.hpp"

#include "EdgeDetection.h"
#include "common/Filter.h"

EdgeDetection::EdgeDetection() {
    this->create();
}

EdgeDetection::~EdgeDetection() {
    this->destroy();
}

EdgeDetection *EdgeDetection::create() {
    const auto ppfXSobel = new const float*[3] {
        new const float[3]{ 1,  2,  1},
        new const float[3]{ 0,  0,  0},
        new const float[3]{-1, -2, -1}
    };
    const auto ppfYSobel = new const float*[3] {
        new const float[3]{-1,  0,  1},
        new const float[3]{-2,  0,  2},
        new const float[3]{-1,  0,  1}
    };
    this->m_pcXSobelFilter = new Filter(ppfXSobel, 3u, 3u, 4.f);
    this->m_pcYSobelFilter = new Filter(ppfYSobel, 3u, 3u, 4.f);
    return this;
}

EdgeDetection *EdgeDetection::destroy() {
    delete this->m_pcXSobelFilter;
    delete this->m_pcYSobelFilter;
    return this;
}

Image *EdgeDetection::detectEdgeBySobelFilter(const Image *image, const PIXEL_TYPE threshold,
                                              const char *xGradientMapOuput, const char *yGradientMapOuput,
                                              const char *gradientMapOuput) const {
    if (image == nullptr || image->getImage() == nullptr || image->getChannel() != NUM_OF_CHANNELS_GRAY) {
        return nullptr;
    }

    const auto pcRawXSobelImage = this->m_pcXSobelFilter->filterImage(image);
    const auto pcXSobelImage = this->m_cImageConverter.normalize(pcRawXSobelImage);
    delete pcRawXSobelImage;
    const auto pcRawYSobelImage = this->m_pcYSobelFilter->filterImage(image);
    const auto pcYSobelImage = this->m_cImageConverter.normalize(pcRawYSobelImage);
    delete pcRawYSobelImage;

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();

    auto pcProcessedImage = (new Image(uiWidth, uiHeight, NUM_OF_CHANNELS_GRAY, image->getDepth(), PaddingType::ALL_BLACK))->create();
    auto pcGradientImage = (new Image(uiWidth, uiHeight, NUM_OF_CHANNELS_GRAY, image->getDepth(), PaddingType::ALL_BLACK))->create();

    auto pppuhProcessedImage = pcProcessedImage->getImage();
    auto pppuhGradientImage = pcGradientImage->getImage();
    const auto pppuhXSobelImage = pcXSobelImage->getImage();
    const auto pppuhYSobelImage = pcYSobelImage->getImage();
    const auto iThresholdSquare = threshold * threshold;
    const auto isOuputGradientMap = gradientMapOuput != nullptr;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto gradientSquare = pppuhXSobelImage[i][j][CHANNEL_GRAY] * pppuhXSobelImage[i][j][CHANNEL_GRAY] +
                pppuhYSobelImage[i][j][CHANNEL_GRAY] * pppuhYSobelImage[i][j][CHANNEL_GRAY];
            pppuhProcessedImage[i][j][CHANNEL_GRAY] = gradientSquare > iThresholdSquare ? MAX_PIXEL_VALUE : 0;
            if (isOuputGradientMap) {
                pppuhGradientImage[i][j][CHANNEL_GRAY] = 0.5 + std::sqrt(gradientSquare);
            }
        }
    }

    if (xGradientMapOuput != nullptr) {
        auto pcNormalizedXSobelImage = this->m_cImageConverter.normalize(pcXSobelImage);
        this->m_cImageIO.writeRawImage(pcNormalizedXSobelImage, xGradientMapOuput);
        delete pcNormalizedXSobelImage;
    }
    if (yGradientMapOuput != nullptr) {
        auto pcNormalizedYSobelImage = this->m_cImageConverter.normalize(pcYSobelImage);
        this->m_cImageIO.writeRawImage(pcNormalizedYSobelImage, yGradientMapOuput);
        delete pcNormalizedYSobelImage;
    }
    if (gradientMapOuput != nullptr) {
        auto pcNormalizedGradientImage = this->m_cImageConverter.normalize(pcGradientImage);
        this->m_cImageIO.writeRawImage(pcNormalizedGradientImage, gradientMapOuput);
        delete pcNormalizedGradientImage;
    }

    delete pcXSobelImage;
    delete pcYSobelImage;
    delete pcGradientImage;
    return pcProcessedImage;
}

Image *EdgeDetection::detectEdgeByCannyDetecteor(const Image *image,
                                                 const PIXEL_TYPE lowThreshold, const PIXEL_TYPE highThreshold) const {
    if (image == nullptr || image->getImage() == nullptr || image->getChannel() != NUM_OF_CHANNELS_GRAY) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto pcInputArray = new cv::Mat(uiHeight, uiWidth, CV_8U, image->writeToArray());
    auto pcOutputArray = new cv::Mat(uiHeight, uiWidth, CV_8U);
    cv::Canny(*pcInputArray, *pcOutputArray, lowThreshold, highThreshold, 3, true);

    pcInputArray->release();
    auto pcProcessedImage = (new Image(uiWidth, uiHeight, NUM_OF_CHANNELS_GRAY, image->getDepth(), PaddingType::ALL_BLACK))->create();
    auto pppuhProcessedImage = pcProcessedImage->getImage();

    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            pppuhProcessedImage[i][j][CHANNEL_GRAY] = pcOutputArray->at<PIXEL_TYPE>(i, j);
        }
    }
    pcOutputArray->release();

    return pcProcessedImage;
}
