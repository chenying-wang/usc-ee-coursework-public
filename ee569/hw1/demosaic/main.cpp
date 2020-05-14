/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>
#include <cstring>

#include "ImageDemosaicing.h"
#include "common/Common.h"
#include "common/Image.h"
#include "common/ImageIO.h"
#include "common/ImageStat.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "mhc"
#define DEFAULT_OUTPUT "./result.raw"
#define DEFAULT_WIDTH "600"
#define DEFAULT_HEIGHT "532"

int main(int argc, char *argv[]) {
#ifdef __DEBUG
    std::cout << "DEBUG IS OPEN\n";
#endif

    ImageIO cImageIO;
    ImageDemosaicing cImageDemosaicing;
    ImageStat cImageStat;


    // Parse options
    const bool bHelp = Utils::getBoolOption(argc, argv, "--help");
    if (bHelp) {
        std::cout << "Usage: See README.md\n";
        return SUCCESS;
    }

    const char *phAlgorithm = Utils::getOption(argc, argv, "-a", DEFAULT_ALGORITHM);
    const char *phInputFilename = Utils::getOption(argc, argv, "-i", nullptr);
    const char *phOutputFilename = Utils::getOption(argc, argv, "-o", DEFAULT_OUTPUT);
    const char *phOriFilename = Utils::getOption(argc, argv, "--ori", nullptr);
    const unsigned int uiWidth = atoi(Utils::getOption(argc, argv, "-w", DEFAULT_WIDTH));
    const unsigned int uiHeight = atoi(Utils::getOption(argc, argv, "-h", DEFAULT_HEIGHT));
    const unsigned int uiChannel = NUM_OF_CHANNELS_GRAY;
    const unsigned int uiDepth = sizeof(PIXEL_TYPE);

    if (phInputFilename == nullptr) {
        std::cerr << "Error: Please specify input file by -i <input_file>\n";
        return ERROR;
    }
    if (uiWidth == 0 || uiHeight == 0) {
        std::cerr << "Error: Unspecified or invalid width or height set by -w <width> -h <height>\n";
        return ERROR;
    }

    std::cout << "================================================================\n";
#ifdef __PROJECT
    std::cout << "\tProject:\t" << __PROJECT << '\n';
#endif
    std::cout << "\tAlgorithm:\t" << phAlgorithm << '\n';
    std::cout << "\tInput:\t\t" << phInputFilename << '\n';
    std::cout << "\tOutput:\t\t" << phOutputFilename << '\n';
    phOriFilename != nullptr && std::cout << "\tOriginal:\t" << phOriFilename << '\n';
    std::cout << "\tWidth:\t\t" << uiWidth << '\n';
    std::cout << "\tHeight:\t\t" << uiHeight << '\n';
    std::cout << "\tChannel:\t" << uiChannel << '\n';
    std::cout << "\tDepth:\t\t" << uiDepth << '\n';
    std::cout << "================================================================\n";

    auto pcInputImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::EVEN_REFLECT);
    cImageIO.readRawImage(pcInputImage, phInputFilename);

    Image *pcRecImage = nullptr;
    if (!std::strcmp(phAlgorithm, "bilinear")) {
        pcRecImage = cImageDemosaicing.bilinearDemosaic(pcInputImage);
    } else if (!std::strcmp(phAlgorithm, "mhc")) {
        pcRecImage = cImageDemosaicing.mhcDemosaic(pcInputImage);
    } else {
        std::cerr << "Error: Unrecognized algorithm found\n";
        delete pcInputImage;
        return ERROR;
    }


    if (phOriFilename != nullptr) {
        auto pcOriImage = new Image(uiWidth, uiHeight, NUM_OF_CHANNELS_RGB, uiDepth, PaddingType::ALL_BLACK);
        cImageIO.readRawImage(pcOriImage, phOriFilename);
        float *pfPSNR = cImageStat.calcPSNR(pcRecImage, pcOriImage);
        std::cout << "\tNoisy PSNR-R:\t" << pfPSNR[CHANNEL_R] << "dB\n";
        std::cout << "\tNoisy PSNR-G:\t" << pfPSNR[CHANNEL_G] << "dB\n";
        std::cout << "\tNoisy PSNR-B:\t" << pfPSNR[CHANNEL_B] << "dB\n";
        delete pcOriImage;
    }

    // Write image to file
    cImageIO.writeRawImage(pcRecImage, phOutputFilename);


    delete pcInputImage;
    delete pcRecImage;
    return SUCCESS;
}
