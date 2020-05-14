/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __IMAGE_MANIPULATION
#define __IMAGE_MANIPULATION

#include <iostream>

#include "common/Image.h"
#include "common/ImageStat.h"

class HistogramManipulation {
private:
    ImageStat cImageStat;
    Image *binarySplitImage(Image *processedImage, const Image *oriImage, unsigned int **cdf,
                            const unsigned int channel, const unsigned int depthInBits,
                            const unsigned int bit, const PIXEL_TYPE mask) const;
public:
    Image *manipulateByTransferFunction(const Image *oriImage, const char *transferFunctionFOutputFilename) const;
    Image *manipulateByBucketFilling(const Image *oriImage) const;
};

#endif
