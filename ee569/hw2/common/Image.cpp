/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <iostream>
#include <cstring>

#include "Image.h"

Image::Image(
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
    m_uiSize(width * height * channel * depth),
    m_pppuhData(nullptr),
    m_uiDataLayout(DATA_LAYOUT_EMPTY) {}

Image::~Image() {
    this->destroy();
}

Image *Image::create() {
    if (this->m_pppuhData != nullptr) {
        return this;
    }

    auto data = new PIXEL_TYPE**[this->m_uiHeight];
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        data[i] = new PIXEL_TYPE*[this->m_uiWidth];
        for (auto j = 0u; j < this->m_uiWidth; ++j) {
            data[i][j] = new PIXEL_TYPE[this->m_uiChannel];
            std::memset(data[i][j], DEFAULT_PIXEL_VALUE, this->m_uiChannel * this->m_uiDepth);
        }
    }
    this->m_pppuhData = data;
    this->m_uiDataLayout = DATA_LAYOUT_3D;
    return this;
}

Image *Image::destroy() {
    if (this->m_pppuhData == nullptr) {
        return this;
    }

    auto data = this->m_pppuhData;
    if (this->m_uiDataLayout == DATA_LAYOUT_3D) {
        for (auto i = 0u; i < this->m_uiHeight; ++i) {
            for (auto j = 0u; j < this->m_uiWidth; ++j) {
                delete[] data[i][j];
            }
            delete[] data[i];
        }
        delete[] data;
    } else if (this->m_uiDataLayout == DATA_LAYOUT_1D_MAPPING) {
        delete[] data[0][0];
    }
    this->m_pppuhData = nullptr;
    this->m_uiDataLayout = DATA_LAYOUT_EMPTY;
    return this;
}

Image *Image::readFromArray(const PIXEL_TYPE* array) {
    if (this->m_pppuhData != nullptr) {
        this->destroy();
    }

    auto copy_array = new PIXEL_TYPE[this->m_uiSize];
    std::memcpy(copy_array, array, this->m_uiSize * sizeof(PIXEL_TYPE));

    auto data = new PIXEL_TYPE**[this->m_uiHeight];
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        data[i] = new PIXEL_TYPE*[this->m_uiWidth];
        for (auto j = 0u; j < this->m_uiWidth; ++j) {
            auto offset = i * this->m_uiWidth + j;
            data[i][j] = copy_array + offset * this->m_uiChannel;

        }
    }
    this->m_pppuhData = data;
    this->m_uiDataLayout = DATA_LAYOUT_1D_MAPPING;
    return this;
}

PIXEL_TYPE *Image::writeToArray() const {
    if (this->m_pppuhData == nullptr) {
        return nullptr;
    }

    if (this->m_uiDataLayout == DATA_LAYOUT_3D) {
        auto array = new PIXEL_TYPE[this->m_uiSize];
        for (auto i = 0u; i < this->m_uiHeight; ++i) {
            for (auto j = 0u; j < this->m_uiWidth; ++j) {
                auto offset = i * this->m_uiWidth + j;
                std::memcpy(array + offset * this->m_uiChannel, this->m_pppuhData[i][j], this->m_uiChannel * this->m_uiDepth);
            }
        }
        return array;
    } else if (this->m_uiDataLayout == DATA_LAYOUT_1D_MAPPING) {
        return this->m_pppuhData[0][0];
    }
    return nullptr;
}

PIXEL_TYPE *Image::getPixel(const int x, const int y) const {
    if (this->m_pppuhData == nullptr) {
        return nullptr;
    }

    const unsigned int uiWidth = this->m_uiWidth, uiHeight = this->m_uiHeight;
    const unsigned int absX = std::abs(x), absY = std::abs(y);

    if (x > 0 && y > 0 && absX < uiHeight && absY < uiWidth) {
        return this->m_pppuhData[x][y];
    } else if (this->m_ePaddingType == PaddingType::ALL_BLACK) {
        auto blackArray = new PIXEL_TYPE[this->m_uiChannel];
        std::memset(blackArray, 0x00, this->m_uiChannel * this->m_uiDepth);
        return blackArray;
    } else if (this->m_ePaddingType == PaddingType::ALL_WHITE) {
        auto whiteArray = new PIXEL_TYPE[this->m_uiChannel];
        std::memset(whiteArray, 0xff, this->m_uiChannel * this->m_uiDepth);
        return whiteArray;
    }

    // PaddingType::EVEN_REFLECT
    auto mappingX = absX % (2 * uiHeight - 1);
    auto mappingY = absY % (2 * uiWidth - 1);
    if (mappingX > uiHeight - 1) {
        mappingX = 2 * uiHeight - 2 - mappingX;
    }
    if (mappingY > uiWidth - 1) {
        mappingY = 2 * uiWidth - 2 - mappingY;
    }
    return this->m_pppuhData[mappingX][mappingY];
}
