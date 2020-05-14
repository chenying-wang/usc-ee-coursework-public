/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __IMAGE_WARP
#define __IMAGE_WARP

#include <iostream>

#include "common/Image.h"

class ImageWarp {
private:
    float calculateFactor(const unsigned int i, const unsigned int j,
                           const unsigned int width, const unsigned int height) const;

public:
    ImageWarp();
    virtual ~ImageWarp();

    Image *warp(const Image *oriImage) const;
    Image *reverseWarp(const Image *warppedImage) const;
};

#endif
