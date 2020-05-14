/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <cstring>
#include <chrono>

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include "FeatureMatcher.h"
#include "common/Common.h"
#include "common/ImageIO.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "keypoints"

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
    const auto phQueryFilename = Utils::getOption(argc, argv, "--query", nullptr);
    const auto phTrainFilename = Utils::getOption(argc, argv, "--train", nullptr);
    const auto phOutputFilename = Utils::getOption(argc, argv, "-o", nullptr);
    const auto phOutputCsvFilename = Utils::getOption(argc, argv, "--output-csv", nullptr);

    if (phQueryFilename == nullptr) {
        std::cerr << "Error: Please specify input file by --query <input_file>\n";
        return ERROR;
    }

    std::cout << "================================================================\n";
#ifdef __PROJECT_NAME
    std::cout << "\tProject:\t" << __PROJECT_NAME << '\n';
#endif
    std::cout << "\tAlgorithm:\t" << phAlgorithm << '\n';
    std::cout << "\tInput:\t\t" << phQueryFilename;
    phTrainFilename != nullptr && std::cout << ", " << phTrainFilename;
    std::cout << '\n';
    phOutputFilename != nullptr && std::cout << "\tOutput:\t\t" << phOutputFilename << '\n';
    std::cout << "================================================================\n";

    const auto start = std::chrono::high_resolution_clock::now();

    ImageIO cImageIO;
    FeatureMatcher cFeatureMatcher;

    // Read Image from file
    auto cQueryImageMat = cv::imread(std::string(phQueryFilename),
        cv::ImreadModes::IMREAD_COLOR);
    cv::cvtColor(cQueryImageMat, cQueryImageMat, cv::ColorConversionCodes::COLOR_BGR2RGB);
    auto cTrainImageMat = cv::Mat();
    if (phTrainFilename != nullptr) {
        cTrainImageMat = cv::imread(std::string(phTrainFilename),
        cv::ImreadModes::IMREAD_COLOR);
        cv::cvtColor(cTrainImageMat, cTrainImageMat, cv::ColorConversionCodes::COLOR_BGR2RGB);
    }

    Image *pcProcessedImage = nullptr;
    if (!std::strcmp(phAlgorithm, "keypoints")) {
        pcProcessedImage = cFeatureMatcher.getKeypoints(&cQueryImageMat);
    } else if (!std::strcmp(phAlgorithm, "largest_keypoints")) {
        pcProcessedImage = cFeatureMatcher.getLargestScaleKeypoints(&cQueryImageMat);
    } else if (!std::strcmp(phAlgorithm, "match")) {
        pcProcessedImage = cFeatureMatcher.match(&cQueryImageMat, &cTrainImageMat);
    } else if (!std::strcmp(phAlgorithm, "match_largest_scale")) {
        pcProcessedImage = cFeatureMatcher.matchLargestScaleKeypoint(&cQueryImageMat, &cTrainImageMat);
    } else if (!std::strcmp(phAlgorithm, "bag_of_words")) {
        pcProcessedImage = cFeatureMatcher.bagOfWords(&cQueryImageMat, phOutputCsvFilename);
    } else if (!std::strcmp(phAlgorithm, "match_bag_of_words")) {
        pcProcessedImage = cFeatureMatcher.matchBagOfWords(&cTrainImageMat, &cQueryImageMat, phOutputCsvFilename);
    } else {
        std::cerr << "Error: Unrecognized algorithm found " << phAlgorithm << '\n';
        cTrainImageMat.release();
        cQueryImageMat.release();
        return ERROR;
    }
    cTrainImageMat.release();
    cQueryImageMat.release();

    // Write image to file
    if (phOutputFilename != nullptr) {
        if (pcProcessedImage == nullptr) {
            std::cerr << "Error: Internal error ocurred\n";
            return ERROR;
        }
        cImageIO.writeRawImage(pcProcessedImage, phOutputFilename);
    }
    if (pcProcessedImage != nullptr) {
        delete pcProcessedImage;
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto timeConsumed =  std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Done with " << timeConsumed << " ms consumed\n\n";

    return SUCCESS;
}
