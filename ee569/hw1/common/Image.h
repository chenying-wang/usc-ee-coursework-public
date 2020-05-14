/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __IMAGE
#define __IMAGE

#include <iostream>

#include "Common.h"

enum class PaddingType {
    ALL_BLACK = 0,
    ALL_WHITE = 1,
    EVEN_REFLECT = 2
};

class Image {
private:
    const unsigned int m_uiWidth;
    const unsigned int m_uiHeight;
    const unsigned int m_uiChannel;
    const unsigned int m_uiDepth;
    const PaddingType m_ePaddingType;
    const unsigned int m_uiSize;
    PIXEL_TYPE ***m_pppuhData;

public:
    Image(
        const unsigned int width,
        const unsigned int height,
        const unsigned int channel,
        const unsigned int depth,
        const PaddingType paddingType) :
    m_uiWidth(width),
    m_uiHeight(height),
    m_uiChannel(channel),
    m_uiDepth(depth),
    m_ePaddingType(paddingType),
    m_uiSize(width * height * channel),
    m_pppuhData(nullptr) {}

    virtual ~Image();

    Image *create();
    Image *destroy();

    Image *readFromArray(const PIXEL_TYPE* array);
    PIXEL_TYPE *writeToArray() const;
    PIXEL_TYPE *getPixel(const int x, const int y) const;

    unsigned int getWidth() const {
        return this->m_uiWidth;
    }

    unsigned int getHeight() const {
        return this->m_uiHeight;
    }

    unsigned int getChannel() const {
        return this->m_uiChannel;
    }

    unsigned int getDepth() const {
        return this->m_uiDepth;
    }

    unsigned int getSize() const {
        return this->m_uiSize;
    }

    PaddingType getPaddingType() const {
        return this->m_ePaddingType;
    }

    PIXEL_TYPE ***getImage() const {
        return this->m_pppuhData;
    }

};

#endif
