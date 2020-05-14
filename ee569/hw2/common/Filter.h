/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#ifndef __FILTER
#define __FILTER

#include <iostream>

#include "Image.h"

class Filter {
private:
    const float **m_ppfWeight;
    const unsigned int m_uiWidth;
    const unsigned int m_uiHeight;
    const float m_iWeightSum;

    float sum(const float **weight,
              const unsigned int width,
              const unsigned int height) const;

    PIXEL_TYPE norm(const float weightedSum) const;

public:
    Filter(const float **weight, const unsigned int width, const unsigned int height);
    Filter(const float **weight, const unsigned int width, const unsigned int height, const float weightSum);

    virtual ~Filter();

    Filter *destroy();

    static PIXEL_TYPE norm(const float weightedSum, const float totalWeight);

    PIXEL_TYPE filterImage(const Image *image, const int x, const int y, const unsigned int channel) const;
    Image *filterImage(const Image *image) const;

    unsigned int getWidth() const {
        return this->m_uiWidth;
    }

    unsigned int getHeight() const {
        return this->m_uiHeight;
    }

};

#endif
