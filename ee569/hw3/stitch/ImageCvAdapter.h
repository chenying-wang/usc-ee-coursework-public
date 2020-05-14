/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __IMAGE_CV_ADAPTER
#define __IMAGE_CV_ADAPTER

#include <iostream>

#include "opencv2/core.hpp"

#include "common/Image.h"

class ImageCvAdapter {
public:
    ImageCvAdapter();
    virtual ~ImageCvAdapter();

    Image *fromCvMat(const cv::Mat *mat) const;
    cv::Mat *toCvMat(const Image *image) const;
};

#endif
