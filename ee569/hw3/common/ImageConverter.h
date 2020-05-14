/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __IMAGE_CONVERTER
#define __IMAGE_CONVERTER

#include <iostream>

#include "Image.h"
#include "Common.h"

class ImageConverter {
public:
    ImageConverter();
    virtual ~ImageConverter();

    Image *duplicate(const Image *image) const;
    Image *normalize(const Image *image) const;
    Image *invert(const Image *image) const;
    Image *binarize(const Image *image, const PIXEL_TYPE threshold) const;
    Image *contertRGBtoY(const Image *image) const;
    Image *contertYtoRGB(const Image *image) const;
    Image *contertRGBtoCMY(const Image *image) const;
    Image *contertCMYtoRGB(const Image *image) const;
};

#endif
