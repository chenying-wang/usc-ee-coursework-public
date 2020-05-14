/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#include <iostream>
#include <cstring>

#include "ImageDenoising.h"
#include "common/Common.h"
#include "common/Image.h"
#include "common/ImageIO.h"
#include "common/ImageStat.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "bilateral"
#define DEFAULT_OUTPUT "./result.raw"
#define DEFAULT_WIDTH "320"
#define DEFAULT_HEIGHT "320"
#define DEFAULT_FILTER_WIDTH "7"
#define DEFAULT_FILTER_HEIGHT "7"
#define DEFAULT_SIGMA_S "0.90"
#define DEFAULT_SIGMA_C "65"
#define DEFAULT_NEIGHBORHOOD_WIDTH "3"
#define DEFAULT_NEIGHBORHOOD_HEIGHT "3"
#define DEFAULT_NLM_H "20"
#define DEFAULT_NLM_A "2.2"

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
    const char *phOriFilename = Utils::getOption(argc, argv, "--ori", nullptr);
    const unsigned int uiWidth = atoi(Utils::getOption(argc, argv, "-w", DEFAULT_WIDTH));
    const unsigned int uiHeight = atoi(Utils::getOption(argc, argv, "-h", DEFAULT_HEIGHT));
    const unsigned int uiChannel = NUM_OF_CHANNELS_GRAY;
    const unsigned int uiDepth = sizeof(PIXEL_TYPE);
    const unsigned int uiFilterWidth = atoi(Utils::getOption(argc, argv, "--filter-width", DEFAULT_FILTER_WIDTH));
    const unsigned int uiFilterHeight = atoi(Utils::getOption(argc, argv, "--filter-height", DEFAULT_FILTER_HEIGHT));
    const char *phNoiseHistogramFilename = Utils::getOption(argc, argv, "--analyze-noise-histogram", nullptr);
    const char *phDiffHistogramFilename = Utils::getOption(argc, argv, "--analyze-diff-histogram", nullptr);

    // for gaussian and bilateral
    const float fSigmaS = atof(Utils::getOption(argc, argv, "--sigma-s", DEFAULT_SIGMA_S));

    // for bilateral only
    const float fSigmaC = atof(Utils::getOption(argc, argv, "--sigma-c", DEFAULT_SIGMA_C));

    // for nlm only
    const unsigned int uiNeighborhoodWidth = atoi(Utils::getOption(argc, argv, "--neighborhood-width", DEFAULT_NEIGHBORHOOD_WIDTH));
    const unsigned int uiNeighborhoodHeight = atoi(Utils::getOption(argc, argv, "--neighborhood-height", DEFAULT_NEIGHBORHOOD_HEIGHT));
    const float fNlmH = atof(Utils::getOption(argc, argv, "--nlm-h", DEFAULT_NLM_H));
    const float fNlmA = atof(Utils::getOption(argc, argv, "--nlm-a", DEFAULT_NLM_A));

    if (phInputFilename == nullptr) {
        std::cerr << "Error: Please specify input file by -i <input_file>\n";
        return ERROR;
    }
    if (uiWidth == 0 || uiHeight == 0) {
        std::cerr << "Error: Unspecified or invalid width or height set by -w <width> -h <height>\n";
        return ERROR;
    }

    std::cout << "================================================================\n";
#ifdef __PROJECT_NAME
    std::cout << "\tProject:\t" << __PROJECT_NAME << '\n';
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

    ImageIO cImageIO;
    ImageDenoising cImageDenoising;
    ImageStat cImageStat;

    // Read Image from file
    auto pcNoisyImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::EVEN_REFLECT);
    cImageIO.readRawImage(pcNoisyImage, phInputFilename);

    Image *pcFilteredImage = nullptr;
    if (!std::strcmp(phAlgorithm, "linear_uniform")) {
        pcFilteredImage = cImageDenoising.uniformFilterImage(pcNoisyImage, uiFilterWidth, uiFilterHeight);
    } else if (!std::strcmp(phAlgorithm, "linear_gaussian")) {
        pcFilteredImage = cImageDenoising.gaussianFilterImage(pcNoisyImage, uiFilterWidth, uiFilterHeight, fSigmaS);
    } else if (!std::strcmp(phAlgorithm, "bilateral")) {
        pcFilteredImage = cImageDenoising.bilateralFilterImage(pcNoisyImage, uiFilterWidth, uiFilterHeight, fSigmaS, fSigmaC);
    } else if (!std::strcmp(phAlgorithm, "nlm")) {
        pcFilteredImage = cImageDenoising.nlmFilterImage(pcNoisyImage, uiFilterWidth, uiFilterHeight,
            uiNeighborhoodWidth, uiNeighborhoodHeight, fNlmH, fNlmA);
    } else {
        std::cerr << "Unrecognized algorithm found " << phAlgorithm << '\n';
        delete pcNoisyImage;
        return ERROR;
    }

    if (phOriFilename != nullptr) {
        auto pcOriImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
        cImageIO.readRawImage(pcOriImage, phOriFilename);
        float *pfNoisyPSNR = cImageStat.calcPSNR(pcNoisyImage, pcOriImage);
        float *pfFilteredPSNR = cImageStat.calcPSNR(pcFilteredImage, pcOriImage);
        std::cout << "\tNoisy PSNR-GRAY:\t" << pfNoisyPSNR[CHANNEL_GRAY] << "dB\n";
        std::cout << "\tFiltered PSNR-GRAY:\t" << pfFilteredPSNR[CHANNEL_GRAY] << "dB\n";

        if (phNoiseHistogramFilename != nullptr) {
            auto ppiResidualPdf = cImageStat.calcResidualPdf(pcNoisyImage, pcOriImage);
            cImageStat.writeCsv(ppiResidualPdf, 2 * MAX_PIXEL_VALUE + 1, uiChannel + 1, phNoiseHistogramFilename);
        }
        if (phDiffHistogramFilename != nullptr) {
            auto ppuiDiffPdf = cImageStat.calcDiffPdf(pcNoisyImage, pcOriImage);
            cImageStat.writeCsv(ppuiDiffPdf, MAX_PIXEL_VALUE + 1, 2 * uiChannel, phDiffHistogramFilename);
        }

        delete pcOriImage;
    }

    // Write image to file
    cImageIO.writeRawImage(pcFilteredImage, phOutputFilename);

    delete pcNoisyImage;
    delete pcFilteredImage;
    return SUCCESS;
}
