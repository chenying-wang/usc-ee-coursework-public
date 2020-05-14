/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <cstring>
#include <chrono>

#include "TextureClassifier.h"
#include "KMeansTextureClassifier.h"
#include "CvTextureClassifier.h"
#include "TextureProcess.h"
#include "common/Common.h"
#include "common/ImageConverter.h"
#include "common/ImageIO.h"
#include "common/Utils.h"

#define DEFAULT_ALGORITHM "pca"
#define DEFAULT_WIDTH "600"
#define DEFAULT_HEIGHT "450"
#define DEFAULT_CHANNEL "1"
#define DEFAULT_DATASET_WIDTH "128"
#define DEFAULT_DATASET_HEIGHT "128"
#define DEFAULT_LAWS_KERNEL "./config/kernel.csv"
#define DEFAULT_PCA "0"
#define DEFAULT_CLUSTER "6"
#define DEFAULT_WINDOW_SIZE "15"

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
    const auto phOutputFilename = Utils::getOption(argc, argv, "-o", nullptr);
    const uint uiWidth = atoi(Utils::getOption(argc, argv, "-w", DEFAULT_WIDTH));
    const uint uiHeight = atoi(Utils::getOption(argc, argv, "-h", DEFAULT_HEIGHT));
    const uint uiChannel = atoi(Utils::getOption(argc, argv, "-c", DEFAULT_CHANNEL));
    const auto phTrainingDatasetFilename = Utils::getOption(argc, argv, "--train", nullptr);
    const auto phTestDatasetFilename = Utils::getOption(argc, argv, "--test", nullptr);
    const uint uiDatasetWidth = atoi(Utils::getOption(argc, argv, "--dataset-width", DEFAULT_DATASET_WIDTH));
    const uint uiDatasetHeight = atoi(Utils::getOption(argc, argv, "--dataset-height", DEFAULT_DATASET_HEIGHT));
    const auto phLawsKernelFilename = Utils::getOption(argc, argv, "--laws-kernel", DEFAULT_LAWS_KERNEL);
    const uint uiPca = atoi(Utils::getOption(argc, argv, "--pca", DEFAULT_PCA));
    const auto phTrainingPcaCsvFilename = Utils::getOption(argc, argv, "--train-pca-csv", nullptr);
    const auto phTestPcaCsvFilename = Utils::getOption(argc, argv, "--test-pca-csv", nullptr);
    const uint uiCluster = atoi(Utils::getOption(argc, argv, "--cluster", DEFAULT_CLUSTER));
    const uint uiWindowSize = atoi(Utils::getOption(argc, argv, "--window-size", DEFAULT_WINDOW_SIZE));
    const auto bOptWindows = Utils::getBoolOption(argc, argv, "--opt-windows");
    const uint uiDepth = sizeof(PIXEL_TYPE);

    std::cout << "================================================================\n";
#ifdef __PROJECT_NAME
    std::cout << "\tProject:\t" << __PROJECT_NAME << '\n';
#endif
    std::cout << "\tAlgorithm:\t" << phAlgorithm << '\n';
    phInputFilename && std::cout << "\tInput:\t\t" << phInputFilename << '\n';
    phOutputFilename && std::cout << "\tOutput:\t\t" << phOutputFilename << '\n';
    phInputFilename && std::cout << "\tWidth:\t\t" << uiWidth << '\n';
    phInputFilename && std::cout << "\tHeight:\t\t" << uiHeight << '\n';
    phInputFilename && std::cout << "\tChannel:\t" << uiChannel << '\n';
    phTrainingDatasetFilename && std::cout << "\tTrain:\t\t" << phTrainingDatasetFilename << '\n';
    phTestDatasetFilename && std::cout << "\tTest:\t\t" << phTestDatasetFilename << '\n';
    phLawsKernelFilename && std::cout << "\tKernels:\t" << phLawsKernelFilename << '\n';
    std::cout << "\tDepth:\t\t" << uiDepth << '\n';
    std::cout << "================================================================\n";

    const auto start = std::chrono::high_resolution_clock::now();

    ImageIO cImageIO;
    const auto pcTextureProcess = new TextureProcess();

    // Read Image from file
    Image *pcOriImage = nullptr;
    if (phInputFilename != nullptr) {
        pcOriImage = new Image(uiWidth, uiHeight, uiChannel, uiDepth, PaddingType::EVEN_REFLECT);
        cImageIO.readRawImage(pcOriImage, phInputFilename);
    }

    Image *pcProcessedImage = nullptr;
    if (!std::strcmp(phAlgorithm, "pca")) {
        pcTextureProcess
            ->load(phLawsKernelFilename)
            ->analyze(phTrainingDatasetFilename, phTestDatasetFilename, uiPca, uiDatasetWidth, uiDatasetHeight,
                phTrainingPcaCsvFilename, phTestPcaCsvFilename);
    } else if (!std::strcmp(phAlgorithm, "k_means")) {
        const auto pcTextureClassifier = new KMeansTextureClassifier(0u, true);
        pcTextureProcess
            ->load(phLawsKernelFilename)
            ->train(pcTextureClassifier, uiPca, phTestDatasetFilename, uiDatasetWidth, uiDatasetHeight)
            ->test(pcTextureClassifier, phTestDatasetFilename, uiDatasetWidth, uiDatasetHeight);
        delete pcTextureClassifier;
    } else if (!std::strcmp(phAlgorithm, "rf")) {
        const auto pcTextureClassifier = new CvTextureClassifier(TEXTURE_OPENCV_MODEL_RF);
        pcTextureProcess
            ->load(phLawsKernelFilename)
            ->train(pcTextureClassifier, uiPca, phTrainingDatasetFilename, uiDatasetWidth, uiDatasetHeight)
            ->test(pcTextureClassifier, phTestDatasetFilename, uiDatasetWidth, uiDatasetHeight);
        delete pcTextureClassifier;
    } else if (!std::strcmp(phAlgorithm, "svm")) {
        const auto pcTextureClassifier = new CvTextureClassifier(TEXTURE_OPENCV_MODEL_SVM);
        pcTextureProcess
            ->load(phLawsKernelFilename)
            ->train(pcTextureClassifier, uiPca, phTrainingDatasetFilename, uiDatasetWidth, uiDatasetHeight)
            ->test(pcTextureClassifier, phTestDatasetFilename, uiDatasetWidth, uiDatasetHeight);
        delete pcTextureClassifier;
    } else if (!std::strcmp(phAlgorithm, "k_means_segment")) {
        const auto pcTextureClassifier = new KMeansTextureClassifier(uiCluster, false);
        pcProcessedImage = pcTextureProcess
            ->load(phLawsKernelFilename)
            ->cluster(pcTextureClassifier, uiPca, pcOriImage, uiCluster, uiWindowSize, bOptWindows);
        delete pcTextureClassifier;
    } else {
        std::cerr << "Error: Unrecognized algorithm found " << phAlgorithm << '\n';
        delete pcOriImage;
        return ERROR;
    }
    delete pcOriImage;
    delete pcTextureProcess;

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
