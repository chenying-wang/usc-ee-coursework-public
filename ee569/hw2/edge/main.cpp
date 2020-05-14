/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <iostream>
#include <cstring>
#include <chrono>

#include "EdgeDetection.h"
#include "common/Common.h"
#include "common/ImageConverter.h"
#include "common/ImageIO.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "sobel"
#define DEFAULT_OUTPUT "./result.raw"
#define DEFAULT_WIDTH "481"
#define DEFAULT_HEIGHT "321"
#define DEFAULT_CHANNEL "3"
#define DEFAULT_THRESHOLD "128"
#define DEFAULT_LOW_THRESHOLD "64"

int main(int argc, char *argv[]) {
#ifdef __DEBUG
    std::cout << "DEBUG IS OPEN\n";
#endif

    // Parse options
    const bool bHelp = Utils::getBoolOption(argc, argv, "--help");
    if (bHelp) {
        std::cout << "Usage: See README.md\n";
        return SUCCESS;
    }

    const auto phAlgorithm = Utils::getOption(argc, argv, "-a", DEFAULT_ALGORITHM);
    const auto phInputFilename = Utils::getOption(argc, argv, "-i", nullptr);
    const auto phOutputFilename = Utils::getOption(argc, argv, "-o", DEFAULT_OUTPUT);
    const unsigned int uiWidth = atoi(Utils::getOption(argc, argv, "-w", DEFAULT_WIDTH));
    const unsigned int uiHeight = atoi(Utils::getOption(argc, argv, "-h", DEFAULT_HEIGHT));
    const unsigned int uiChannel = atoi(Utils::getOption(argc, argv, "-c", DEFAULT_CHANNEL));
    const PIXEL_TYPE uhThreshold = atoi(Utils::getOption(argc, argv, "--threshold", DEFAULT_THRESHOLD));
    const auto phXGradientMapOuput = Utils::getOption(argc, argv, "--x-gradient-output", nullptr);
    const auto phYGradientMapOuput = Utils::getOption(argc, argv, "--y-gradient-output", nullptr);
    const auto phGradientMapOuput = Utils::getOption(argc, argv, "--gradient-output", nullptr);
    const PIXEL_TYPE uhLowThreshold = atoi(Utils::getOption(argc, argv, "--low-threshold", DEFAULT_LOW_THRESHOLD));
    const unsigned int uiDepth = sizeof(PIXEL_TYPE);

    if (phInputFilename == nullptr) {
        std::cerr << "Error: Please specify input file by -i <input_file>\n";
        return ERROR;
    }
    if (uiWidth == 0 || uiHeight == 0) {
        std::cerr << "Error: Unspecified or invalid width or height set by -w <width> -h <height>\n";
        return ERROR;
    }
    if (uiChannel != 1 && uiChannel != 3) {
        std::cerr << "Error: Unspecified or invalid channel set by -c <num_of_channels>\n";
        return ERROR;
    }

    std::cout << "================================================================\n";
#ifdef __PROJECT_NAME
    std::cout << "\tProject:\t" << __PROJECT_NAME << '\n';
#endif
    std::cout << "\tAlgorithm:\t" << phAlgorithm << '\n';
    std::cout << "\tInput:\t\t" << phInputFilename << '\n';
    std::cout << "\tOutput:\t\t" << phOutputFilename << '\n';
    std::cout << "\tWidth:\t\t" << uiWidth << '\n';
    std::cout << "\tHeight:\t\t" << uiHeight << '\n';
    std::cout << "\tChannel:\t" << uiChannel << '\n';
    std::cout << "\tDepth:\t\t" << uiDepth << '\n';
    std::cout << "================================================================\n";

    const auto start = std::chrono::high_resolution_clock::now();

    ImageIO cImageIO;
    ImageConverter cImageConverter;
    EdgeDetection cEdgeDetection;

    // Read Image from file
    auto pcOriImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::EVEN_REFLECT);
    cImageIO.readRawImage(pcOriImage, phInputFilename);

    if (uiChannel == NUM_OF_CHANNELS_RGB) {
        auto pcOriImageTemp = pcOriImage;
        pcOriImage = cImageConverter.contertRGBtoY(pcOriImage);
        delete pcOriImageTemp;
    }

    Image *pcProcessedImage = nullptr;
    if (!std::strcmp(phAlgorithm, "sobel")) {
        pcProcessedImage = cEdgeDetection.detectEdgeBySobelFilter(pcOriImage, uhThreshold,
                                                                  phXGradientMapOuput, phYGradientMapOuput,
                                                                  phGradientMapOuput);
    } else if (!std::strcmp(phAlgorithm, "canny")) {
        pcProcessedImage = cEdgeDetection.detectEdgeByCannyDetecteor(pcOriImage,
                                                                     uhLowThreshold, uhThreshold);
    } else {
        std::cerr << "Unrecognized algorithm found " << phAlgorithm << '\n';
        return ERROR;
    }

    // Write image to file
    cImageIO.writeRawImage(pcProcessedImage, phOutputFilename);

    delete pcOriImage;
    delete pcProcessedImage;

    const auto end = std::chrono::high_resolution_clock::now();
    const auto timeConsumed =  std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Done with " << timeConsumed << " ms consumed\n\n";

    return SUCCESS;
}
