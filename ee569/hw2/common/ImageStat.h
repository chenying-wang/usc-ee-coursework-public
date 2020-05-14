/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#ifndef __IMAGE_STAT
#define __IMAGE_STAT

#include <iostream>

#include "Image.h"
#include "Common.h"

class ImageStat {
public:
    ImageStat();
    virtual ~ImageStat();

    float *calcPSNR(const Image *filteredImage, const Image *oriImage) const;
    unsigned int **calcPdf(const Image *image) const;
    unsigned int **calcCdfFromPdf(const Image *image, unsigned int **ppuiPdf) const;
    unsigned int **calcCdf(const Image *image) const;
    int **calcResidualPdf(const Image *noisyImage, const Image *oriImage) const;
    unsigned int **calcDiffPdf(const Image *noisyImage, const Image *image) const;
    void writeCsv(unsigned int **matrix, const unsigned int length, const unsigned int width, const char *filename) const;
    void writeCsv(int **matrix, const unsigned int length, const unsigned int width, const char *filename) const;
};

#endif
