/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#ifndef __IMAGE_HALFTONING
#define __IMAGE_HALFTONING

#include <iostream>

#include "DiffusionMatrix.h"
#include "common/Image.h"
#include "common/ImageConverter.h"

class ImageHalftoning {
private:
    ImageConverter m_cImageConverter;

    DiffusionMatrix *m_pcFSDiffusionMatrix;
    DiffusionMatrix *m_pcJJNDiffusionMatrix;
    DiffusionMatrix *m_pcStuckiDiffusionMatrix;

    PIXEL_TYPE **getDitheringMatrix(const unsigned int size) const;
    Image *halftoneByErrorDiffusion(const Image *image, const PIXEL_TYPE threshold,
                                    const DiffusionMatrix *diffusionMatrix) const;
    Image *halftoneByErrorDiffusionMBVQ(Image *image, const DiffusionMatrix *diffusionMatrix) const;

public:
    ImageHalftoning();
    virtual ~ImageHalftoning();

    ImageHalftoning *create();
    ImageHalftoning *destroy();

    Image *halftoneByFixedThreshold(const Image *image, const PIXEL_TYPE threshold) const;
    Image *halftoneByRandomThreshold(const Image *image) const;
    Image *halftoneByDitheringMatrix(const Image *image, const unsigned int matrixSize) const;
    Image *halftoneByFSErrorDiffusion(const Image *image, const PIXEL_TYPE threshold) const;
    Image *halftoneByJJNErrorDiffusion(const Image *image, const PIXEL_TYPE threshold) const;
    Image *halftoneByStuckiErrorDiffusion(const Image *image, const PIXEL_TYPE threshold) const;
    Image *halftoneByErrorDiffusionMBVQwithFS(Image *image) const;
    Image *halftoneByErrorDiffusionMBVQwithJJN(Image *image) const;
};

#endif
