/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#include <iostream>
#include <cstring>
#include <chrono>

#include "ImageHalftoning.h"
#include "common/Common.h"
#include "common/ImageConverter.h"
#include "common/ImageIO.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "fixed"
#define DEFAULT_OUTPUT "./result.raw"
#define DEFAULT_WIDTH "750"
#define DEFAULT_HEIGHT "500"
#define DEFAULT_CHANNEL "1"
#define DEFAULT_THRESHOLD "128"
#define DEFAULT_DITHERING_MATRIX_SIZE "1"

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
    const unsigned int uiDitheringMatrixSize = atoi(Utils::getOption(argc, argv,
        "--dithering-matrix-size", DEFAULT_DITHERING_MATRIX_SIZE));
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
    ImageHalftoning cImageHalftoning;

    // Read Image from file
    auto pcOriImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::ALL_BLACK);
    cImageIO.readRawImage(pcOriImage, phInputFilename);

    Image *pcProcessedImage = nullptr;
    if (!std::strcmp(phAlgorithm, "fixed")) {
        pcProcessedImage = cImageHalftoning.halftoneByFixedThreshold(pcOriImage, uhThreshold);
    } else if (!std::strcmp(phAlgorithm, "random")) {
        pcProcessedImage = cImageHalftoning.halftoneByRandomThreshold(pcOriImage);
    } else if (!std::strcmp(phAlgorithm, "dithering_matrix")) {
        pcProcessedImage = cImageHalftoning.halftoneByDitheringMatrix(pcOriImage, uiDitheringMatrixSize);
    } else if (!std::strcmp(phAlgorithm, "error_diffusion_fs")) {
        pcProcessedImage = cImageHalftoning.halftoneByFSErrorDiffusion(pcOriImage, uhThreshold);
    } else if (!std::strcmp(phAlgorithm, "error_diffusion_jjn")) {
        pcProcessedImage = cImageHalftoning.halftoneByJJNErrorDiffusion(pcOriImage, uhThreshold);
    } else if (!std::strcmp(phAlgorithm, "error_diffusion_stucki")) {
        pcProcessedImage = cImageHalftoning.halftoneByStuckiErrorDiffusion(pcOriImage, uhThreshold);
    } else if (!std::strcmp(phAlgorithm, "error_diffusion_mbvq_fs")) {
        pcProcessedImage = cImageHalftoning.halftoneByErrorDiffusionMBVQwithFS(pcOriImage);
    } else if (!std::strcmp(phAlgorithm, "error_diffusion_mbvq_jjn")) {
        pcProcessedImage = cImageHalftoning.halftoneByErrorDiffusionMBVQwithJJN(pcOriImage);
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
