/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __IMAGE_DENOISING
#define __IMAGE_DENOISING

#include <iostream>

#include "common/Filter.h"

class ImageDenoising {
public:
    ImageDenoising();
    virtual ~ImageDenoising();

    Image *uniformFilterImage(const Image *oriImage, const unsigned int filterWidth, const unsigned int filterHeight) const;
    Image *gaussianFilterImage(const Image *oriImage, const unsigned int filterWidth, const unsigned int filterHeight,
                               const float sigma) const;
    Image *bilateralFilterImage(const Image *oriImage, const unsigned int filterWidth, const unsigned int filterHeight,
                                const float sigmaS, const float sigmaC) const;
    Image *nlmFilterImage(const Image *oriImage, const unsigned int filterWidth, const unsigned int filterHeight,
                          const unsigned int neighborhoodWidth, const unsigned int neighborhoodHeight,
                          const float h, const float a) const;
    Image *filterImage(const Image *oriImage, const Filter *filter,
                       const unsigned int filterWidth, const unsigned int filterHeight) const;

};

#endif
