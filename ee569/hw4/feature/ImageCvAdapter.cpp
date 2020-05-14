/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>

#include "opencv2/core.hpp"

#include "ImageCvAdapter.h"
#include "common/Image.h"

ImageCvAdapter::ImageCvAdapter() {}

ImageCvAdapter::~ImageCvAdapter() {}

Image *ImageCvAdapter::fromCvMat(const cv::Mat *mat) const {
    if (mat == nullptr) {
        return nullptr;
    }
    const auto cSize = mat->size;
    const auto iDims = cSize.dims();
    if (!iDims) {
        return nullptr;
    }
    const unsigned int uiWidth = cSize.p[1];
    const unsigned int uiHeight = cSize.p[0];
    const unsigned int uiChannel = mat->channels();

    const auto iCvDepth = mat->depth();
    auto uiDepth = 1u;
    if (iCvDepth == CV_8U) {
        uiDepth = 1u;
    } else if (iCvDepth == CV_16U) {
        uiDepth = 2u;
    } else {
        std::cerr << "Error: Unsupported depth from cv::Mat\n";
        return nullptr;
    }

    return (new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK))->readFromArray(mat->data);
}

cv::Mat *ImageCvAdapter::toCvMat(const Image *image) const {
    if (image == nullptr || image->getDepth() != 1) {
        return nullptr;
    }
    const auto uiDepth = image->getDepth();
    auto iCvDepth = CV_8U;
    if (uiDepth == 1) {
        iCvDepth = CV_8U;
    } else if (uiDepth == 2) {
        iCvDepth = CV_16U;
    } else {
        std::cerr << "Error: Unsupported depth to cv::Mat\n";
        return nullptr;
    }
    return new cv::Mat(image->getHeight(), image->getWidth(), CV_MAKETYPE(iCvDepth, image->getChannel()), image->writeToArray());
}
