/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
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
    void appendToRawImage(const Image *image, const char *filename) const;
    void clean(const char *filename) const;
};

#endif
