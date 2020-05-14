/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>
#include <cstring>
#include <chrono>

#include "ImageStitching.h"
#include "common/Common.h"
#include "common/ImageIO.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "stitch"
#define DEFAULT_OUTPUT "./result.raw"
#define DEFAULT_WIDTH "480"
#define DEFAULT_HEIGHT "720"
#define DEFAULT_CHANNEL "3"

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
    const auto phLeftInputFilename = Utils::getOption(argc, argv, "--left", nullptr);
    const auto phMiddleInputFilename = Utils::getOption(argc, argv, "--middle", nullptr);
    const auto phRightInputFilename = Utils::getOption(argc, argv, "--right", nullptr);
    const auto phOutputFilename = Utils::getOption(argc, argv, "-o", DEFAULT_OUTPUT);
    const unsigned int uiWidth = atoi(Utils::getOption(argc, argv, "-w", DEFAULT_WIDTH));
    const unsigned int uiHeight = atoi(Utils::getOption(argc, argv, "-h", DEFAULT_HEIGHT));
    const unsigned int uiChannel = atoi(Utils::getOption(argc, argv, "-c", DEFAULT_CHANNEL));
    const char *phMatchesLMFilename = Utils::getOption(argc, argv, "--lm-matches", nullptr);
    const char *phMatchesMRFilename = Utils::getOption(argc, argv, "--mr-matches", nullptr);
    const char *phBestMatchesLMFilename = Utils::getOption(argc, argv, "--lm-best-matches", nullptr);
    const char *phBestMatchesMRFilename = Utils::getOption(argc, argv, "--mr-best-matches", nullptr);
    const unsigned int uiDepth = sizeof(PIXEL_TYPE);

    if (phLeftInputFilename == nullptr ||
        phMiddleInputFilename == nullptr ||
        phRightInputFilename == nullptr) {
        std::cerr << "Error: Please specify input file by --left/middle/right <input_file>\n";
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
    std::cout << "\tInput:\t\t" << phLeftInputFilename << ", "
              << phMiddleInputFilename << ", "
              << phRightInputFilename << '\n';
    std::cout << "\tOutput:\t\t" << phOutputFilename << '\n';
    std::cout << "\tWidth:\t\t" << uiWidth << '\n';
    std::cout << "\tHeight:\t\t" << uiHeight << '\n';
    std::cout << "\tChannel:\t" << uiChannel << '\n';
    std::cout << "\tDepth:\t\t" << uiDepth << '\n';
    std::cout << "================================================================\n";

    const auto start = std::chrono::high_resolution_clock::now();

    ImageIO cImageIO;
    ImageStitching cImageStitching;

    // Read Image from file
    const auto pcLeftImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    cImageIO.readRawImage(pcLeftImage, phLeftInputFilename);
    const auto pcMiddleImage = pcLeftImage->dumpStructure();
    cImageIO.readRawImage(pcMiddleImage, phMiddleInputFilename);
    const auto pcRightImage = pcLeftImage->dumpStructure();
    cImageIO.readRawImage(pcRightImage, phRightInputFilename);

    Image *pcProcessedImage = nullptr;
    if (!std::strcmp(phAlgorithm, "stitch")) {
        pcProcessedImage = cImageStitching.stitch(pcLeftImage, pcMiddleImage, pcRightImage,
            phMatchesLMFilename, phMatchesMRFilename,
            phBestMatchesLMFilename, phBestMatchesMRFilename);
    } else {
        std::cerr << "Error: Unrecognized algorithm found " << phAlgorithm << '\n';
        delete pcLeftImage;
        delete pcMiddleImage;
        delete pcRightImage;
        return ERROR;
    }
    delete pcLeftImage;
    delete pcMiddleImage;
    delete pcRightImage;

    if (pcProcessedImage == nullptr) {
        std::cerr << "Error: Internal error ocurred\n";
        return ERROR;
    }

    // Write image to file
    cImageIO.writeRawImage(pcProcessedImage, phOutputFilename);

    delete pcProcessedImage;

    const auto end = std::chrono::high_resolution_clock::now();
    const auto timeConsumed =  std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Done with " << timeConsumed << " ms consumed\n\n";

    return SUCCESS;
}
