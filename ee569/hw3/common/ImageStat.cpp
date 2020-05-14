/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>
#include <fstream>
#include <cmath>

#include "ImageStat.h"

ImageStat::ImageStat() {}

ImageStat::~ImageStat() {}

float *ImageStat::calcPSNR(const Image *filteredImage, const Image *oriImage) const {
    if (filteredImage == nullptr ||
        oriImage == nullptr ||
        filteredImage->getImage() == nullptr ||
        oriImage->getImage() == nullptr ||
        filteredImage->getWidth() != oriImage->getWidth() ||
        filteredImage->getHeight() != oriImage->getHeight() ||
        filteredImage->getChannel() != oriImage->getChannel() ||
        filteredImage->getDepth() != oriImage->getDepth()) {
        return nullptr;
    }

    float *pfPSNR = new float[filteredImage->getChannel()];
    unsigned int *puiMSE = new unsigned int[filteredImage->getChannel()];
    for (auto i = 0u; i < filteredImage->getChannel(); ++i) {
        pfPSNR[i] = 0;
        puiMSE[i] = 0;
    }

    PIXEL_TYPE ***pppuhFilteredImage = filteredImage->getImage(), ***pppuhOriImage = oriImage->getImage();
    for (auto i = 0u; i < filteredImage->getHeight(); ++i) {
        for (auto j = 0u; j < filteredImage->getWidth(); ++j) {
            for (auto k = 0u; k < filteredImage->getChannel(); ++k) {
                int difference = pppuhFilteredImage[i][j][k] - pppuhOriImage[i][j][k];
                puiMSE[k] += difference * difference;
            }
        }
    }

    unsigned int uiMax = (1 << (filteredImage->getDepth() << 3)) - 1;
    for (auto i = 0u; i < filteredImage->getChannel(); ++i) {
        pfPSNR[i] = 10 * log10(1. * uiMax * uiMax * filteredImage->getSize() / puiMSE[i]);
    }
    return pfPSNR;
}

unsigned int **ImageStat::calcPdf(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        std::cerr << "Image is null or not initialized\n";
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    const auto ppuiPdf = new unsigned int*[MAX_PIXEL_VALUE + 1];
    for (auto i = 0u; i <= MAX_PIXEL_VALUE; ++i) {
        ppuiPdf[i] = new unsigned int[uiChannel];
        for (auto j = 0u; j < uiChannel; ++j) {
            ppuiPdf[i][j] = 0;
        }
    }

    auto pppuhImage = image->getImage();
    for (auto i = 0u; i < image->getHeight(); ++i) {
        for (auto j = 0u; j < image->getWidth(); ++j) {
            for (auto k = 0u; k < uiChannel; ++k) {
                ppuiPdf[pppuhImage[i][j][k]][k] += 1;
            }
        }
    }
    return ppuiPdf;
}

unsigned int **ImageStat::calcCdfFromPdf(const Image *image, unsigned int **ppuiPdf) const {
    if (image == nullptr || image->getImage() == nullptr) {
        std::cerr << "Image is null or not initialized\n";
        return nullptr;
    }

    const auto uiChannel = image->getChannel();
    auto ppuiCdf = new unsigned int*[MAX_PIXEL_VALUE + 1];
    ppuiCdf[0] = new unsigned int[uiChannel];
    for (auto i = 0u; i < uiChannel; ++i) {
        ppuiCdf[0][i] += ppuiPdf[0][i];
    }
    for (auto i = 1u; i <= MAX_PIXEL_VALUE; ++i) {
        ppuiCdf[i] = new unsigned int[uiChannel];
        for (auto j = 0u; j < uiChannel; ++j) {
            ppuiCdf[i][j] = ppuiCdf[i - 1][j] + ppuiPdf[i][j];
        }
    }
    return ppuiCdf;
}

unsigned int **ImageStat::calcCdf(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        std::cerr << "Image is null or not initialized\n";
    }

    const auto ppuiPdf = this->calcPdf(image);
    return this->calcCdfFromPdf(image, ppuiPdf);
}

int **ImageStat::calcResidualPdf(const Image *noisyImage, const Image *oriImage) const {
    if (noisyImage == nullptr ||
        oriImage == nullptr ||
        noisyImage->getImage() == nullptr ||
        oriImage->getImage() == nullptr ||
        noisyImage->getWidth() != oriImage->getWidth() ||
        noisyImage->getHeight() != oriImage->getHeight() ||
        noisyImage->getChannel() != oriImage->getChannel() ||
        noisyImage->getDepth() != oriImage->getDepth()) {
        return nullptr;
    }

    const auto iLength = 2 * MAX_PIXEL_VALUE + 1;
    const auto uiChannel = noisyImage->getChannel();
    auto ppiResidualPdf = new int*[iLength];
    for (auto i = 0u; i < iLength; ++i) {
        ppiResidualPdf[i] = new int[uiChannel + 1];
        ppiResidualPdf[i][0] = i > MAX_PIXEL_VALUE ? i - iLength: i;
        for (auto j = 0u; j < uiChannel; ++j) {
            ppiResidualPdf[i][j + 1] = 0;
        }
    }

    auto pppuhNoisyImage = noisyImage->getImage();
    auto pppuhOriImage = oriImage->getImage();
    for (auto i = 0u; i < noisyImage->getHeight(); ++i) {
        for (auto j = 0u; j < noisyImage->getWidth(); ++j) {
            for (auto k = 0u; k < noisyImage->getChannel(); ++k) {
                int difference = pppuhNoisyImage[i][j][k] - pppuhOriImage[i][j][k];
                if (difference < 0) {
                    difference += iLength;
                }
                ppiResidualPdf[difference][k + 1] += 1;
            }
        }
    }
    return ppiResidualPdf;
}

unsigned int **ImageStat::calcDiffPdf(const Image *noisyImage, const Image *oriImage) const {
    if (noisyImage == nullptr ||
        oriImage == nullptr ||
        noisyImage->getImage() == nullptr ||
        oriImage->getImage() == nullptr ||
        noisyImage->getWidth() != oriImage->getWidth() ||
        noisyImage->getHeight() != oriImage->getHeight() ||
        noisyImage->getChannel() != oriImage->getChannel() ||
        noisyImage->getDepth() != oriImage->getDepth()) {
        return nullptr;
    }

    const auto uiChannel = noisyImage->getChannel();
    auto ppuiDiffPdf = new unsigned int*[MAX_PIXEL_VALUE + 1];
    for (auto i = 0u; i <= MAX_PIXEL_VALUE; ++i) {
        ppuiDiffPdf[i] = new unsigned int[2 * uiChannel];
        for (auto j = 0u; j < 2 * uiChannel; ++j) {
            ppuiDiffPdf[i][j] = 0;
        }
    }

    auto pppuhNoisyImage = noisyImage->getImage();
    auto pppuhOriImage = oriImage->getImage();
    for (auto i = 0u; i < noisyImage->getHeight(); ++i) {
        for (auto j = 0u; j < noisyImage->getWidth(); ++j) {
            for (auto k = 0u; k < uiChannel; ++k) {
                if (pppuhNoisyImage[i][j][k] - pppuhOriImage[i][j][k]) {
                    ppuiDiffPdf[pppuhNoisyImage[i][j][k]][k] += 1;
                    ppuiDiffPdf[pppuhOriImage[i][j][k]][uiChannel + k] += 1;
                }
            }
        }
    }
    return ppuiDiffPdf;
}

void ImageStat::writeCsv(unsigned int **matrix, const unsigned int length, const unsigned int width,
                         const char *filename) const {
    if (filename == nullptr) {
        std::cerr << "Filename is null\n";
        return;
    }
    std::ofstream csv;
    csv.open(filename, std::ios::out);
    for (auto i = 0u; i < length; ++i) {
        csv << i;
        for (auto j = 0u; j < width; ++j) {
            csv << ',' << matrix[i][j];
        }
        csv << '\n';
    }
    csv.close();
}

void ImageStat::writeCsv(unsigned int *matrix, const unsigned int length, const char *filename) const {
    if (filename == nullptr) {
        std::cerr << "Filename is null\n";
        return;
    }
    std::ofstream csv;
    csv.open(filename, std::ios::out);
    for (auto i = 0u; i < length; ++i) {
        csv << i << ',' << matrix[i] << '\n';
    }
    csv.close();
}

void ImageStat::writeCsv(int **matrix, const unsigned int length, const unsigned int width,
                         const char *filename) const {
    if (filename == nullptr) {
        std::cerr << "Filename is null\n";
        return;
    }
    std::ofstream csv;
    csv.open(filename);
    for (auto i = 0u; i < length; ++i) {
        csv << matrix[i][0];
        for (auto j = 1u; j < width; ++j) {
            csv << ',' << matrix[i][j];
        }
        csv << '\n';
    }
    csv.close();
}
