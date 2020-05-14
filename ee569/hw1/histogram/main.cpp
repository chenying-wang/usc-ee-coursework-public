/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>
#include <cstring>

#include "HistogramManipulation.h"
#include "common/Common.h"
#include "common/Image.h"
#include "common/ImageIO.h"
#include "common/ImageStat.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "bucket_filling"
#define DEFAULT_OUTPUT "./result.raw"
#define DEFAULT_WIDTH "400"
#define DEFAULT_HEIGHT "560"
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

    const char *phAlgorithm = Utils::getOption(argc, argv, "-a", DEFAULT_ALGORITHM);
    const char *phInputFilename = Utils::getOption(argc, argv, "-i", nullptr);
    const char *phOutputFilename = Utils::getOption(argc, argv, "-o", DEFAULT_OUTPUT);
    const unsigned int uiWidth = atoi(Utils::getOption(argc, argv, "-w", DEFAULT_WIDTH));
    const unsigned int uiHeight = atoi(Utils::getOption(argc, argv, "-h", DEFAULT_HEIGHT));
    const unsigned int uiChannel = atoi(Utils::getOption(argc, argv, "-c", DEFAULT_CHANNEL));
    const char *phOriPdfOutputFilename = Utils::getOption(argc, argv, "--ori-pdf-output", nullptr);
    const char *phTransferFunctionOutputFilename = Utils::getOption(argc, argv, "--transfer-function-output", nullptr);
    const char *phCdfOuputFilename = Utils::getOption(argc, argv, "--cdf-output", nullptr);
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

    ImageIO cImageIO;
    ImageStat cImageStat;
    HistogramManipulation cHistogramManipulation;

    // Read Image from file
    auto pcOriImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    cImageIO.readRawImage(pcOriImage, phInputFilename);

    Image *pcProcessedImage = nullptr;
    if (!std::strcmp(phAlgorithm, "transfer_function")) {
        pcProcessedImage = cHistogramManipulation.manipulateByTransferFunction(pcOriImage, phTransferFunctionOutputFilename);
    } else if (!std::strcmp(phAlgorithm, "bucket_filling")) {
        pcProcessedImage = cHistogramManipulation.manipulateByBucketFilling(pcOriImage);
    } else {
        std::cerr << "Unrecognized algorithm found " << phAlgorithm << '\n';
        return ERROR;
    }

    if (phOriPdfOutputFilename != nullptr) {
        auto ppuiOriCdf = cImageStat.calcPdf(pcOriImage);
        cImageStat.writeCsv(ppuiOriCdf, MAX_PIXEL_VALUE + 1, uiChannel, phOriPdfOutputFilename);
    }

    if (phCdfOuputFilename != nullptr) {
        auto ppuiCdf = cImageStat.calcCdf(pcProcessedImage);
        cImageStat.writeCsv(ppuiCdf, MAX_PIXEL_VALUE + 1, uiChannel, phCdfOuputFilename);
    }

    // Write image to file
    cImageIO.writeRawImage(pcProcessedImage, phOutputFilename);

    delete pcOriImage;
    delete pcProcessedImage;
    return SUCCESS;
}
