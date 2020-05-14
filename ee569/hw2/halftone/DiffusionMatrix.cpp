/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include "DiffusionMatrix.h"

DiffusionMatrix::DiffusionMatrix(
    const int **matrix,
    const unsigned int width,
    const unsigned int height) :
    m_ppiMatrix(matrix),
    m_uiWidth(width),
    m_uiHeight(height) {
    this->create();
}

DiffusionMatrix::~DiffusionMatrix() {
    this->destroy();
}

DiffusionMatrix *DiffusionMatrix::create() {
    if (this->m_ppiMatrix == nullptr || this->m_ppiMatrix[0][0] != 0) {
        this->m_iMatrixSum = 0;
        this->m_puiCenter = nullptr;
        return this;
    }

    auto iMatrixSum = 0;
    auto bCenterFound = false;
    unsigned int *puiCenter = nullptr;
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        for (auto j = 0u; j < this->m_uiWidth; ++j) {
            if (this->m_ppiMatrix[i][j] == 0) {
                continue;
            }
            iMatrixSum += this->m_ppiMatrix[i][j];
            if (!bCenterFound) {
                if (j == 0) {
                    puiCenter = new unsigned int[2]{i - 1, this->m_uiWidth - 1};
                } else {
                    puiCenter = new unsigned int[2]{i, j - 1};
                }
                bCenterFound = true;
            }
        }
    }
    this->m_iMatrixSum = iMatrixSum;
    if (bCenterFound) {
        this->m_puiCenter = puiCenter;
    } else {
        this->m_puiCenter = nullptr;
    }
    return this;
}

DiffusionMatrix *DiffusionMatrix::destroy() {
    for (auto i = 0u; i < this->m_uiHeight; ++i) {
        delete[] this->m_ppiMatrix[i];
    }
    delete[] this->m_ppiMatrix;
    delete[] this->m_puiCenter;
    return this;
}

Image *DiffusionMatrix::diffuseError(Image *image, const int error, const bool flip,
                                     const unsigned int x, const unsigned int y,
                                     const unsigned int channel) const {
    if (image == nullptr || image->getImage() == nullptr ||
        this->m_iMatrixSum == 0 || this->m_puiCenter == nullptr) {
        return image;
    }

    if (error == 0 || std::abs(error) > MAX_PIXEL_VALUE) {
        return image;
    }
    auto pppuhImage = image->getImage();
    for (auto i = this->m_puiCenter[0]; i < this->m_uiHeight; ++i) {
        auto compX = x + i - this->m_puiCenter[0];
        for (auto j = 0u; j < this->m_uiWidth; ++j) {
            if (this->m_ppiMatrix[i][j] == 0) {
                continue;
            }
            int compensation = (this->m_iMatrixSum / 2 + this->m_ppiMatrix[i][j] * error) / this->m_iMatrixSum;
            auto compY = flip ? y - j + this->m_puiCenter[1] : y + j - this->m_puiCenter[1];
            if (compX >= image->getHeight() || compY >= image->getWidth()) {
                continue;
            }
            pppuhImage[compX][compY][channel] += compensation;
        }
    }
    return image;
}

bool DiffusionMatrix::isValid() const {
    return this->m_iMatrixSum != 0 && this->m_puiCenter != nullptr && this->m_ppiMatrix != nullptr;
}
