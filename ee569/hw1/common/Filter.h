/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __FILTER
#define __FILTER

#include <iostream>

#include "Image.h"

class Filter {
private:
    float **m_ppiWeight;
    const unsigned int m_uiWidth;
    const unsigned int m_uiHeight;
    const float m_iWeightSum;

    float sum(float **weight,
                      const unsigned int width,
                      const unsigned int height) const;

    PIXEL_TYPE norm(const float weightedSum) const;

public:
    Filter(
        float **weight,
        const unsigned int width,
        const unsigned int height) :
    m_ppiWeight(weight),
    m_uiWidth(width),
    m_uiHeight(height),
    m_iWeightSum(this->sum(weight, width, height)) {}

    virtual ~Filter();

    Filter *destroy();

    static PIXEL_TYPE norm(const float weightedSum, const float totalWeight);

    PIXEL_TYPE filterImage(const Image *image, const int x, const int y, const unsigned int channel) const;

    unsigned int getWidth() const {
        return this->m_uiWidth;
    }

    unsigned int getHeight() const {
        return this->m_uiHeight;
    }

};

#endif
