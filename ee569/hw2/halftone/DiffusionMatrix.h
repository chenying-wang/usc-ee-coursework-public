/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#ifndef __DIFFUSION_MATRIX
#define __DIFFUSION_MATRIX

#include <iostream>

#include "common/Image.h"

class DiffusionMatrix
{
private:
    const int **m_ppiMatrix;
    const unsigned int m_uiWidth;
    const unsigned int m_uiHeight;
    int m_iMatrixSum;
    unsigned int *m_puiCenter;

public:
    DiffusionMatrix(const int **matrix, const unsigned int width, const unsigned int height);
    virtual ~DiffusionMatrix();

    DiffusionMatrix *create();
    DiffusionMatrix *destroy();

    Image *diffuseError(Image *image, const int error, const bool flip,
                        const unsigned int x, const unsigned int y,
                        const unsigned int channel) const;
    bool isValid() const;

    unsigned int getWidth() const {
        return this->m_uiWidth;
    }

    unsigned int getHeight() const {
        return this->m_uiHeight;
    }
};


#endif