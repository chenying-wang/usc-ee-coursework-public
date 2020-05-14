/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __IMAGE_IO
#define __IMAGE_IO

#include <iostream>

#include "Image.h"
#include "Common.h"

class ImageIO {
public:
    ImageIO();
    virtual ~ImageIO();

    Image *readRawImage(Image *image, const char *filename) const;
    void writeRawImage(const Image *image, const char *filename) const;
};

#endif
