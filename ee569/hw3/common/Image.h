/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __IMAGE
#define __IMAGE

#include <iostream>
#include <functional>
#include <thread>

#define DATA_LAYOUT_EMPTY 0
#define DATA_LAYOUT_3D 1
#define DATA_LAYOUT_1D_MAPPING 2

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
    unsigned int m_uiDataLayout;

public:
    Image(
        const unsigned int width,
        const unsigned int height,
        const unsigned int channel,
        const unsigned int depth,
        const PaddingType paddingType);

    virtual ~Image();

    Image *create();
    Image *destroy();

    Image *readFromArray(const PIXEL_TYPE* array);
    PIXEL_TYPE *writeToArray() const;
    PIXEL_TYPE *getPixel(const int x, const int y) const;
    Image *dumpStructure() const;
    void rasterScan(const std::function<void(unsigned int, unsigned int)>& fn) const;
    Image *rasterMap(const std::function<PIXEL_TYPE *(unsigned int, unsigned int)>& fn) const;
    void concurrentScan(const std::function<void(unsigned int, unsigned int)>& fn,
                        const uint8_t threads = std::thread::hardware_concurrency() << 1) const;
    Image *concurrentMap(const std::function<PIXEL_TYPE *(unsigned int, unsigned int)>& fn,
                         const uint8_t threads = std::thread::hardware_concurrency() << 1) const;

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

    unsigned int getDataLayout() const {
        return this->m_uiDataLayout;
    }

};

#endif
