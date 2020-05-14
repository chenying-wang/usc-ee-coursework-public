/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __TEXTURE_PROCESS
#define __TEXTURE_PROCESS

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

#include "TextureClassifier.h"
#include "TextureDataset.h"
#include "common/CSVReader.h"
#include "common/Filter.h"
#include "common/ImageIO.h"

class TextureProcess {
private:
    CSVReader m_cCsvReader;
    ImageIO m_cImageIO;
    std::vector<const Filter *> *m_pppcFilter = nullptr;
    std::vector<std::string> *m_psFilterName = nullptr;
    std::vector<const Filter *> *m_pppcSymFilter = nullptr;
    std::vector<std::string> *m_psSymFilterName = nullptr;

    TextureDataset loadDataset(const char *datasetFilename, const uint width, const uint height) const;
    std::vector<float> extractFeatures(const Image *image) const;
    std::vector<float> **extractFeaturesByPixel(const Image *image, const uint windowSize) const;
    std::vector<float> **norm(std::vector<float> **features,
                              const uint width, const uint height,
                              const std::vector<float> weight) const;
    uint maxAccuracy(const std::vector<std::string> estimations, const std::vector<std::string> labels,
                     std::vector<std::string> psEstimation = std::vector<std::string>(),
                     std::vector<std::string> psLabel = std::vector<std::string>(),
                     std::unordered_set<uint> usedLabel = std::unordered_set<uint>()) const;

public:
    TextureProcess();
    virtual ~TextureProcess();

    TextureProcess *load(const char *kernelFilename);
    const TextureProcess *train(TextureClassifier *textureClassifier, const uint pca,
                                const char *trainingDatasetFilename,
                                const uint width, const uint height) const;
    const TextureProcess *test(const TextureClassifier *textureClassifier,
                               const char *testDatasetFilename,
                               const uint width, const uint height) const;
    const TextureProcess *analyze(const char *trainingDatasetFilename, const char *testDatasetFilename,
                                  const uint pca, const uint width, const uint height,
                                  const char *trainPcaCsv, const char *testPcaCsv) const;
    Image *cluster(TextureClassifier *textureClassifier, const uint pca,
                   const Image *image, const uint k, const uint windowSize,
                   const bool optWindows) const;
};

#endif
