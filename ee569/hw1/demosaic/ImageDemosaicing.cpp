/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>

#include "ImageDemosaicing.h"
#include "common/Filter.h"
#include "common/Image.h"

ImageDemosaicing::ImageDemosaicing() {}

ImageDemosaicing::~ImageDemosaicing() {}

Image *ImageDemosaicing::bilinearDemosaic(const Image *oriImage) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr || oriImage->getChannel() != NUM_OF_CHANNELS_GRAY) {
        return nullptr;
    }

    std::cout << "Bilinear Demosaicing Start\n";
    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = NUM_OF_CHANNELS_RGB;
    const auto uiDepth = oriImage->getDepth();
    const auto pcRecImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    auto pppuhRecImage = pcRecImage->create()->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto pos = (i & 0x1) << 1 | (j & 0x1);
            auto puhRecPixel = pppuhRecImage[i][j];

            auto uhOriPixel = oriImage->getPixel(i, j)[CHANNEL_GRAY];

            auto uhOriUpPixel = oriImage->getPixel(i - 1, j)[CHANNEL_GRAY];
            auto uhOriDownPixel = oriImage->getPixel(i + 1, j)[CHANNEL_GRAY];
            auto uhOriLeftPixel = oriImage->getPixel(i, j - 1)[CHANNEL_GRAY];
            auto uhOriRightPixel = oriImage->getPixel(i, j + 1)[CHANNEL_GRAY];

            auto uhOriULPixel = oriImage->getPixel(i - 1, j - 1)[CHANNEL_GRAY];
            auto uhOriURPixel = oriImage->getPixel(i - 1, j + 1)[CHANNEL_GRAY];
            auto uhOriDLPixel = oriImage->getPixel(i + 1, j - 1)[CHANNEL_GRAY];
            auto uhOriDRPixel = oriImage->getPixel(i + 1, j + 1)[CHANNEL_GRAY];
            if (pos == 0) {
                // Green in a Red row
                puhRecPixel[CHANNEL_G] = uhOriPixel;
                puhRecPixel[CHANNEL_R] = Filter::norm(uhOriLeftPixel + uhOriRightPixel, 2);
                puhRecPixel[CHANNEL_B] = Filter::norm(uhOriUpPixel + uhOriDownPixel, 2);
            } else if (pos == 1) {
                // Red
                puhRecPixel[CHANNEL_R] = uhOriPixel;
                puhRecPixel[CHANNEL_G] = Filter::norm(uhOriUpPixel + uhOriDownPixel + uhOriLeftPixel + uhOriRightPixel, 4);
                puhRecPixel[CHANNEL_B] = Filter::norm(uhOriULPixel + uhOriURPixel + uhOriDLPixel + uhOriDRPixel, 4);
            } else if (pos == 2) {
                // Blue
                puhRecPixel[CHANNEL_B] = uhOriPixel;
                puhRecPixel[CHANNEL_R] = Filter::norm(uhOriULPixel + uhOriURPixel + uhOriDLPixel + uhOriDRPixel, 4);
                puhRecPixel[CHANNEL_G] = Filter::norm(uhOriUpPixel + uhOriDownPixel + uhOriLeftPixel + uhOriRightPixel, 4);
            } else {
                // Green in a Blue row
                puhRecPixel[CHANNEL_G] = uhOriPixel;
                puhRecPixel[CHANNEL_R] = Filter::norm(uhOriUpPixel + uhOriDownPixel, 2);
                puhRecPixel[CHANNEL_B] = Filter::norm(uhOriLeftPixel + uhOriRightPixel, 2);
            }
        }
    }
    std::cout << "Bilinear Demosaicing Done\n";
    return pcRecImage;
}

Image *ImageDemosaicing::mhcDemosaic(const Image *oriImage) const {
    if (oriImage == nullptr || oriImage->getImage() == nullptr || oriImage->getChannel() != NUM_OF_CHANNELS_GRAY) {
        return nullptr;
    }

    std::cout << "MHC Demosaicing Start\n";
    const auto uiWidth = oriImage->getWidth();
    const auto uiHeight = oriImage->getHeight();
    const auto uiChannel = NUM_OF_CHANNELS_RGB;
    const auto uiDepth = oriImage->getDepth();
    Image *pcRecImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    auto pppuhRecImage = pcRecImage->create()->getImage();
    for (auto i = 0u; i < uiHeight; ++i) {
        for (auto j = 0u; j < uiWidth; ++j) {
            auto pos = (i & 0x1) << 1 | (j & 0x1);
            auto puhRecPixel = pppuhRecImage[i][j];

            auto uhOriPixel = oriImage->getPixel(i, j)[CHANNEL_GRAY];

            auto uhOriUpPixel = oriImage->getPixel(i - 1, j)[CHANNEL_GRAY];
            auto uhOriDownPixel = oriImage->getPixel(i + 1, j)[CHANNEL_GRAY];
            auto uhOriLeftPixel = oriImage->getPixel(i, j - 1)[CHANNEL_GRAY];
            auto uhOriRightPixel = oriImage->getPixel(i, j + 1)[CHANNEL_GRAY];

            auto uhOriULPixel = oriImage->getPixel(i - 1, j - 1)[CHANNEL_GRAY];
            auto uhOriURPixel = oriImage->getPixel(i - 1, j + 1)[CHANNEL_GRAY];
            auto uhOriDLPixel = oriImage->getPixel(i + 1, j - 1)[CHANNEL_GRAY];
            auto uhOriDRPixel = oriImage->getPixel(i + 1, j + 1)[CHANNEL_GRAY];

            auto uhOriUUPixel = oriImage->getPixel(i - 2, j)[CHANNEL_GRAY];
            auto uhOriDDPixel = oriImage->getPixel(i + 2, j)[CHANNEL_GRAY];
            auto uhOriLLPixel = oriImage->getPixel(i, j - 2)[CHANNEL_GRAY];
            auto uhOriRRPixel = oriImage->getPixel(i, j + 2)[CHANNEL_GRAY];
            if (pos == 0) {
                // Green in a Red row
                puhRecPixel[CHANNEL_G] = uhOriPixel;
                puhRecPixel[CHANNEL_R] = Filter::norm(10*uhOriPixel + 8*uhOriLeftPixel + 8*uhOriRightPixel -
                    2*uhOriULPixel - 2*uhOriURPixel - 2*uhOriDLPixel - 2*uhOriDRPixel +
                    uhOriUUPixel + uhOriDDPixel - 2*uhOriLLPixel - 2*uhOriRRPixel, 16);
                puhRecPixel[CHANNEL_B] = Filter::norm(10*uhOriPixel + 8*uhOriUpPixel + 8*uhOriDownPixel -
                    2*uhOriULPixel - 2*uhOriURPixel - 2*uhOriDLPixel - 2*uhOriDRPixel -
                    2*uhOriUUPixel - 2*uhOriDDPixel + uhOriLLPixel + uhOriRRPixel, 16);
            } else if (pos == 1) {
                // Red
                puhRecPixel[CHANNEL_R] = uhOriPixel;
                puhRecPixel[CHANNEL_G] = Filter::norm(4*uhOriPixel +
                    2*uhOriUpPixel + 2*uhOriDownPixel + 2*uhOriLeftPixel + 2*uhOriRightPixel -
                    uhOriUUPixel - uhOriDDPixel - uhOriLLPixel - uhOriRRPixel, 8);
                puhRecPixel[CHANNEL_B] = Filter::norm(12*uhOriPixel +
                    4*uhOriULPixel + 4*uhOriURPixel + 4*uhOriDLPixel + 4*uhOriDRPixel -
                    3*uhOriUUPixel - 3*uhOriDDPixel - 3*uhOriLLPixel - 3*uhOriRRPixel, 16);
            } else if (pos == 2) {
                // Blue
                puhRecPixel[CHANNEL_B] = uhOriPixel;
                puhRecPixel[CHANNEL_R] = Filter::norm(12*uhOriPixel +
                    4*uhOriULPixel + 4*uhOriURPixel + 4*uhOriDLPixel + 4*uhOriDRPixel -
                    3*uhOriUUPixel - 3*uhOriDDPixel - 3*uhOriLLPixel - 3*uhOriRRPixel, 16);
                puhRecPixel[CHANNEL_G] = Filter::norm(4*uhOriPixel +
                    2*uhOriUpPixel + 2*uhOriDownPixel + 2*uhOriLeftPixel + 2*uhOriRightPixel -
                    uhOriUUPixel - uhOriDDPixel - uhOriLLPixel - uhOriRRPixel, 8);
            } else {
                // Green in a Blue row
                puhRecPixel[CHANNEL_G] = uhOriPixel;
                puhRecPixel[CHANNEL_R] = Filter::norm(10*uhOriPixel + 8*uhOriUpPixel + 8*uhOriDownPixel -
                    2*uhOriULPixel - 2*uhOriURPixel - 2*uhOriDLPixel - 2*uhOriDRPixel -
                    2*uhOriUUPixel - 2*uhOriDDPixel + uhOriLLPixel + uhOriRRPixel, 16);
                puhRecPixel[CHANNEL_B] = Filter::norm(10*uhOriPixel + 8*uhOriLeftPixel + 8*uhOriRightPixel -
                    2*uhOriULPixel - 2*uhOriURPixel - 2*uhOriDLPixel - 2*uhOriDRPixel +
                    uhOriUUPixel + uhOriDDPixel - 2*uhOriLLPixel - 2*uhOriRRPixel, 16);
            }
        }
    }
    std::cout << "MHC Demosaicing Done\n";
    return pcRecImage;
}
