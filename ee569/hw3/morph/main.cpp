/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>
#include <cstring>
#include <chrono>

#include "MorphologicalFilter.h"
#include "common/Common.h"
#include "common/ImageConverter.h"
#include "common/ImageIO.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "shrink"
#define DEFAULT_OUTPUT "./result.raw"
#define DEFAULT_WIDTH "558"
#define DEFAULT_HEIGHT "558"
#define DEFAULT_CHANNEL "1"
#define DEFAULT_THRESHOLD "127"
#define DEFAULT_ITERATION "1000"

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
    const PIXEL_TYPE uhThread = atoi(Utils::getOption(argc, argv, "--threshold", DEFAULT_THRESHOLD));
    const auto phStage1MasksFilename = Utils::getOption(argc, argv, "--stage1-masks", nullptr);
    const auto phStage2MasksFilename = Utils::getOption(argc, argv, "--stage2-masks", nullptr);
    const unsigned int uiIteration = atoi(Utils::getOption(argc, argv, "--iteration", DEFAULT_ITERATION));
    const auto phSizeCsvFilename = Utils::getOption(argc, argv, "--count-csv", nullptr);
    const auto phTrackFilename = Utils::getOption(argc, argv, "--track", nullptr);
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
    if (phStage1MasksFilename == nullptr || phStage2MasksFilename == nullptr) {
        std::cerr << "Error: Unspecified masks filename\n";
        return ERROR;
    }
    if (uiIteration == 0) {
        std::cerr << "Error: Iteration must be positive set by --iteration <iteration>\n";
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
    const MorphologicalFilter *pcMorphologicalFilter = new MorphologicalFilter(phStage1MasksFilename, phStage2MasksFilename);

    // Read Image from file
    auto pcOriImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    cImageIO.readRawImage(pcOriImage, phInputFilename);

    if (uiChannel == NUM_OF_CHANNELS_RGB) {
        auto pcOriImageTemp = pcOriImage;
        pcOriImage = cImageConverter.contertRGBtoY(pcOriImage);
        delete pcOriImageTemp;
    }

    const auto pcOriImageTemp = pcOriImage;
    pcOriImage = cImageConverter.binarize(pcOriImage, uhThread);
    delete pcOriImageTemp;

    Image *pcProcessedImage = nullptr;
    if (!std::strcmp(phAlgorithm, "shrink")) {
        pcProcessedImage = pcMorphologicalFilter->shrink(pcOriImage, uiIteration, phTrackFilename);
    } else if (!std::strcmp(phAlgorithm, "thin")) {
        pcProcessedImage = pcMorphologicalFilter->thin(pcOriImage, uiIteration, phTrackFilename);
    } else if (!std::strcmp(phAlgorithm, "skeletonize")) {
        pcProcessedImage = pcMorphologicalFilter->skeletonize(pcOriImage, uiIteration, phTrackFilename);
    } else if (!std::strcmp(phAlgorithm, "count")) {
        pcProcessedImage = pcMorphologicalFilter->count(pcOriImage, uiIteration, phSizeCsvFilename);
    } else if (!std::strcmp(phAlgorithm, "count_dfs")) {
        pcProcessedImage = pcMorphologicalFilter->countByDfs(pcOriImage, phSizeCsvFilename);
    } else if (!std::strcmp(phAlgorithm, "pcb_hole")) {
        pcProcessedImage = pcMorphologicalFilter->pcbHole(pcOriImage, uiIteration);
    } else if (!std::strcmp(phAlgorithm, "pcb_pathway")) {
        pcProcessedImage = pcMorphologicalFilter->pcbPathway(pcOriImage, uiIteration, phTrackFilename);
    } else if (!std::strcmp(phAlgorithm, "defect")) {
        pcProcessedImage = pcMorphologicalFilter->detectDefect(pcOriImage, uiIteration, phTrackFilename);
    } else {
        std::cerr << "Error: Unrecognized algorithm found " << phAlgorithm << '\n';
        delete pcMorphologicalFilter;
        delete pcOriImage;
        return ERROR;
    }
    delete pcOriImage;
    delete pcMorphologicalFilter;

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
