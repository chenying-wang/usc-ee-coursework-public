/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __IMAGE_DEMOSAICING
#define __IMAGE_DEMOSAICING

#include <iostream>

#include "common/Filter.h"
#include "common/Image.h"

class ImageDemosaicing {
public:
    ImageDemosaicing();
    ~ImageDemosaicing();

    Image *bilinearDemosaic(const Image *oriImage) const;
    Image *mhcDemosaic(const Image *oriImage) const;
};

#endif
