/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>
#include <cstring>

#include "Image.h"

Image::~Image() {
    this->destroy();
}

Image *Image::create() {
    if (this->m_pppuhData != nullptr) {
        return this;
    }

    auto data = new uint8_t**[this->m_uiHeight];
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        data[i] = new uint8_t*[this->m_uiWidth];
        for (auto j = 0u; j < this->m_uiWidth; ++j) {
            data[i][j] = new uint8_t[this->m_uiChannel];
            std::memset(data[i][j], DEFAULT_PIXEL_VALUE, this->m_uiChannel * this->m_uiDepth);
        }
    }
    this->m_pppuhData = data;
    return this;
}

Image *Image::destroy() {
    if (this->m_pppuhData == nullptr) {
        return this;
    }

    auto data = this->m_pppuhData;
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        for (auto j = 0u; j < m_uiWidth; ++j) {
            delete[] data[i][j];
        }
        delete[] data[i];
    }
    delete[] data;
    this->m_pppuhData = nullptr;
    return this;
}

Image *Image::readFromArray(const PIXEL_TYPE* array) {
    if (this->m_pppuhData == nullptr) {
        this->create();
    }

    const auto uiBytesPerPixel = this->m_uiChannel * this->m_uiDepth;
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        for (auto j = 0u; j < this->m_uiWidth; ++j) {
            auto offset = i * this->m_uiWidth + j;
            std::memcpy(this->m_pppuhData[i][j], array + offset * this->m_uiChannel, uiBytesPerPixel);
        }
    }
    return this;
}

PIXEL_TYPE *Image::writeToArray() const {
    if (this->m_pppuhData == nullptr) {
        return nullptr;
    }

    auto array = new uint8_t[this->m_uiSize];
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        for (auto j = 0u; j < this->m_uiWidth; ++j) {
            auto offset = i * this->m_uiWidth + j;
            std::memcpy(array + offset * this->m_uiChannel, this->m_pppuhData[i][j], this->m_uiChannel * this->m_uiDepth);
        }
    }
    return array;
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
        auto blackArray = new uint8_t[this->m_uiChannel];
        std::memset(blackArray, 0x00, this->m_uiChannel * this->m_uiDepth);
        return blackArray;
    } else if (this->m_ePaddingType == PaddingType::ALL_WHITE) {
        auto whiteArray = new uint8_t[this->m_uiChannel];
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
