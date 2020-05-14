/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <fstream>
#include <cstring>

#include "ImageHalftoning.h"

ImageHalftoning::ImageHalftoning() {
    this->create();
}

ImageHalftoning::~ImageHalftoning() {
    this->destroy();
}

ImageHalftoning *ImageHalftoning::create() {
    this->m_pcFSDiffusionMatrix = new DiffusionMatrix(new const int*[2] {
        new const int[3]{0, 0, 7},
        new const int[3]{3, 5, 1}
    }, 3, 2);
    this->m_pcJJNDiffusionMatrix = new DiffusionMatrix(new const int*[3] {
        new const int[5]{0, 0, 0, 7, 5},
        new const int[5]{3, 5, 7, 5, 3},
        new const int[5]{1, 3, 5, 3, 1}
    }, 5, 3);
    this->m_pcStuckiDiffusionMatrix = new DiffusionMatrix(new const int*[3] {
        new const int[5]{0, 0, 0, 8, 4},
        new const int[5]{2, 4, 8, 4, 2},
        new const int[5]{1, 2, 4, 2, 1}
    }, 5, 3);
    return this;
}

ImageHalftoning *ImageHalftoning::destroy() {
    return this;
}

Image *ImageHalftoning::halftoneByFixedThreshold(const Image *image, const PIXEL_TYPE threshold) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    const auto uiDepth = image->getDepth();
    auto pcProcessedImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK))->create();

    const auto pppuhOriImage = image->getImage();
    auto pppuhProcessedImage = pcProcessedImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            for (auto k = 0u; k < uiChannel; ++k) {
                if (pppuhOriImage[i][j][k] > threshold) {
                    pppuhProcessedImage[i][j][k] = MAX_PIXEL_VALUE;
                } else {
                    pppuhProcessedImage[i][j][k] = 0x0;
                }
            }
        }
    }
    return pcProcessedImage;
}

Image *ImageHalftoning::halftoneByRandomThreshold(const Image *image) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    const auto uiDepth = image->getDepth();
    auto pcProcessedImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK))->create();

    std::ifstream urandomInput;
    urandomInput.open("/dev/urandom", std::ios::in | std::ios::binary);

    const auto pppuhOriImage = image->getImage();
    auto pppuhProcessedImage = pcProcessedImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            for (auto k = 0u; k < uiChannel; ++k) {
                const auto size = sizeof(PIXEL_TYPE);
                char *uhRandom = new char[size];
                urandomInput.read(uhRandom, size);
                PIXEL_TYPE uhThreshold = 0x0;
                for (auto l = 0u; l < size; ++l) {
                    uhThreshold |= (uhRandom[l] << (l << 3));
                }
                if (pppuhOriImage[i][j][k] > uhThreshold) {
                    pppuhProcessedImage[i][j][k] = MAX_PIXEL_VALUE;
                } else {
                    pppuhProcessedImage[i][j][k] = 0x0;
                }
            }
        }
    }
    urandomInput.close();
    return pcProcessedImage;
}

Image *ImageHalftoning::halftoneByDitheringMatrix(const Image *image, const unsigned int matrixSize) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    const auto uiDepth = image->getDepth();
    auto pcProcessedImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK))->create();

    PIXEL_TYPE **ppuhDitheringMatrix = this->getDitheringMatrix(matrixSize);
    const unsigned int mask = (1 << matrixSize) - 1;

    const auto pppuhOriImage = image->getImage();
    auto pppuhProcessedImage = pcProcessedImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto uhThreshold = ppuhDitheringMatrix[i & mask][j & mask];
            for (auto k = 0u; k < uiChannel; ++k) {
                if (pppuhOriImage[i][j][k] > uhThreshold) {
                    pppuhProcessedImage[i][j][k] = MAX_PIXEL_VALUE;
                } else {
                    pppuhProcessedImage[i][j][k] = 0x0;
                }
            }
        }
    }
    return pcProcessedImage;
}

Image *ImageHalftoning::halftoneByFSErrorDiffusion(const Image *image, const PIXEL_TYPE threshold) const {
    return this->halftoneByErrorDiffusion(image, threshold, this->m_pcFSDiffusionMatrix);
}

Image *ImageHalftoning::halftoneByJJNErrorDiffusion(const Image *image, const PIXEL_TYPE threshold) const {
    return this->halftoneByErrorDiffusion(image, threshold, this->m_pcJJNDiffusionMatrix);
}

Image *ImageHalftoning::halftoneByStuckiErrorDiffusion(const Image *image, const PIXEL_TYPE threshold) const {
    return this->halftoneByErrorDiffusion(image, threshold, this->m_pcStuckiDiffusionMatrix);
}

Image *ImageHalftoning::halftoneByErrorDiffusionMBVQwithFS(Image *image) const {
    return this->halftoneByErrorDiffusionMBVQ(image, this->m_pcFSDiffusionMatrix);
}
Image *ImageHalftoning::halftoneByErrorDiffusionMBVQwithJJN(Image *image) const {
    return this->halftoneByErrorDiffusionMBVQ(image, this->m_pcJJNDiffusionMatrix);
}

PIXEL_TYPE **ImageHalftoning::getDitheringMatrix(const unsigned int matrixSize) const {
    const unsigned int length = 1 << matrixSize;
    PIXEL_TYPE **ppuhDitheringMatrix = new PIXEL_TYPE*[length];
    for (auto i = 0u; i < length; ++i) {
        ppuhDitheringMatrix[i] = new PIXEL_TYPE[length];
    }

    unsigned int size = (1 << (matrixSize << 1));
    for (auto i = 0u; i < length; ++i) {
        for (auto j = 0u; j < length; ++j) {
            unsigned int weight = 0;
            for (auto k = 0u; k < matrixSize; ++k) {
                unsigned char idx = ((i >> k & 0x1) << 1 ) | (j >> k & 0x1);
                weight |= ((0x1 + idx) & 0x3) << ((matrixSize - 1 - k) << 1);
            }
            weight = 0x1 | weight << 1;
            ppuhDitheringMatrix[i][j] = (weight * MAX_PIXEL_VALUE + size) /
                (size << 1);
        }
    }
    return ppuhDitheringMatrix;
}

Image *ImageHalftoning::halftoneByErrorDiffusion(const Image *image, const PIXEL_TYPE threshold,
                                                 const DiffusionMatrix *diffusionMatrix) const {
    if (image == nullptr || image->getImage() == nullptr) {
        return nullptr;
    } else if (diffusionMatrix == nullptr || !diffusionMatrix->isValid()) {
        return this->halftoneByFixedThreshold(image, threshold);
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    const auto uiDepth = image->getDepth();

    const Image *pcOriImage = image;
    auto pcProcessedImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK))->create();

    if (uiChannel == NUM_OF_CHANNELS_RGB) {
        pcOriImage = this->m_cImageConverter.contertRGBtoCMY(image);
    }

    const auto pppuhOriImage = pcOriImage->getImage();
    auto pppuhProcessedImage = pcProcessedImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            std::memcpy(pppuhProcessedImage[i][j], pppuhOriImage[i][j], sizeof(PIXEL_TYPE) * uiChannel);
        }
    }

    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto _j = 0u; _j < uiWidth; ++_j) {
            auto j = _j;
            bool flip = i & 0x1;
            if (flip) {
                j = uiWidth - 1 - j;
            }
            for (auto k = 0u; k < uiChannel; ++k) {
                auto error = 0;
                if (pppuhProcessedImage[i][j][k] > threshold) {
                    error = pppuhProcessedImage[i][j][k] - MAX_PIXEL_VALUE;
                    pppuhProcessedImage[i][j][k] = MAX_PIXEL_VALUE;
                } else {
                    error = pppuhProcessedImage[i][j][k];
                    pppuhProcessedImage[i][j][k] = 0x0;
                }
                pcProcessedImage = diffusionMatrix->diffuseError(pcProcessedImage, error, flip, i, j, k);
            }
        }
    }

    if (uiChannel == NUM_OF_CHANNELS_CMY) {
        auto pcProcessedImageTemp = pcProcessedImage;
        pcProcessedImage = this->m_cImageConverter.contertCMYtoRGB(pcProcessedImage);
        delete pcProcessedImageTemp;
        delete pcOriImage;
    }

    return pcProcessedImage;
}

Image *ImageHalftoning::halftoneByErrorDiffusionMBVQ(Image *image, const DiffusionMatrix *diffusionMatrix) const {
    if (image == nullptr || image->getImage() == nullptr || diffusionMatrix == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_RGB) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto uiChannel = image->getChannel();
    const auto uiDepth = image->getDepth();
    auto pcProcessedImage = (new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK))->create();

    const auto pppuhOriImage = image->getImage();
    auto pppuhProcessedImage = pcProcessedImage->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            std::memcpy(pppuhProcessedImage[i][j], pppuhOriImage[i][j], sizeof(PIXEL_TYPE) * uiChannel);
        }
    }

    const PIXEL_TYPE uhHalfPixelValue = (MAX_PIXEL_VALUE >> 1) + 1;
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto _j = 0u; _j < uiWidth; ++_j) {
            auto j = _j;
            bool flip = i & 0x1;
            if (flip) {
                j = uiWidth - 1 - j;
            }
            const PIXEL_TYPE uhOriRed = pppuhProcessedImage[i][j][CHANNEL_R];
            const PIXEL_TYPE uhOriGreen = pppuhProcessedImage[i][j][CHANNEL_G];
            const PIXEL_TYPE uhOriBlue = pppuhProcessedImage[i][j][CHANNEL_B];

            const PIXEL_TYPE uhRed = pppuhProcessedImage[i][j][CHANNEL_R];
            const PIXEL_TYPE uhGreen = pppuhProcessedImage[i][j][CHANNEL_G];
            const PIXEL_TYPE uhBlue = pppuhProcessedImage[i][j][CHANNEL_B];
            delete[] pppuhProcessedImage[i][j];
            pppuhProcessedImage[i][j] = nullptr;
            if (uhOriRed + uhOriGreen + uhOriBlue <= MAX_PIXEL_VALUE) {
                // KRGB
                if (uhRed > uhHalfPixelValue) {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, 0};
                } else if (uhGreen > uhHalfPixelValue) {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, 0};
                } else if (uhBlue > uhHalfPixelValue) {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, 0, MAX_PIXEL_VALUE};
                } else {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, 0, 0};
                }
            } else if (uhOriRed + uhOriGreen + uhOriBlue >= MAX_PIXEL_VALUE << 1) {
                // CMYW
                if (uhRed <= uhHalfPixelValue) {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, MAX_PIXEL_VALUE};
                } else if (uhGreen <= uhHalfPixelValue) {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, MAX_PIXEL_VALUE};
                } else if (uhBlue <= uhHalfPixelValue) {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, MAX_PIXEL_VALUE, 0};
                } else {
                    pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, MAX_PIXEL_VALUE, MAX_PIXEL_VALUE};
                }
            } else if (uhOriRed + uhOriGreen < MAX_PIXEL_VALUE ||
                ((uhOriRed < uhOriGreen) && (uhOriRed + uhOriGreen == MAX_PIXEL_VALUE))) {
                if (uhGreen + uhBlue <= MAX_PIXEL_VALUE) {
                    // RGBM
                    if (uhGreen > uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, 0};
                    } else if (uhRed <= uhHalfPixelValue && uhRed < uhBlue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, 0, MAX_PIXEL_VALUE};
                    } else if (uhBlue <= uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, 0};
                    } else {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, MAX_PIXEL_VALUE};
                    }
                } else {
                    // CMGB
                    if (uhBlue < uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, 0};
                    } else if (uhRed >= uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, MAX_PIXEL_VALUE};
                    } else if (uhGreen >= uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, MAX_PIXEL_VALUE};
                    } else {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, 0, MAX_PIXEL_VALUE};
                    }
                }
            } else {
                if (uhGreen + uhBlue < MAX_PIXEL_VALUE) {
                    // RGMY
                    if (uhBlue > uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, MAX_PIXEL_VALUE};
                    } else if (uhRed <= uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, 0};
                    } else if (uhGreen <= uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, 0};
                    } else {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, MAX_PIXEL_VALUE, 0};
                    }
                } else {
                    // MYGC
                    if (uhGreen <= uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, 0, MAX_PIXEL_VALUE};
                    } else if (uhRed > uhHalfPixelValue && uhRed > uhBlue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{MAX_PIXEL_VALUE, MAX_PIXEL_VALUE, 0};
                    } else if (uhBlue > uhHalfPixelValue) {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, MAX_PIXEL_VALUE};
                    }   else {
                        pppuhProcessedImage[i][j] = new PIXEL_TYPE[NUM_OF_CHANNELS_RGB]{0, MAX_PIXEL_VALUE, 0};
                    }
                }
            }
            const int iRedError = uhRed - pppuhProcessedImage[i][j][CHANNEL_R];
            const int iGreenError = uhGreen - pppuhProcessedImage[i][j][CHANNEL_G];
            const int iBlueError = uhBlue - pppuhProcessedImage[i][j][CHANNEL_B];
            pcProcessedImage = diffusionMatrix->diffuseError(pcProcessedImage, iRedError, flip, i, j, CHANNEL_R);
            pcProcessedImage = diffusionMatrix->diffuseError(pcProcessedImage, iGreenError, flip, i, j, CHANNEL_G);
            pcProcessedImage = diffusionMatrix->diffuseError(pcProcessedImage, iBlueError, flip, i, j, CHANNEL_B);
        }
    }
    return pcProcessedImage;
}
