/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __PCA_ANALYZER
#define __PCA_ANALYZER

#include <iostream>

#include "TextureClassifier.h"
#include "TextureDataset.h"
#include "common/CSVWriter.h"

class FeatureAnalyzer : public TextureClassifier {
private:
    const std::vector<std::string> *m_ppsFilterName;
    const std::vector<std::string> *m_ppsSymFilterName;
    CSVWriter m_cCSVWriter;

    void analyzeFeatures(const std::unordered_multimap<std::string, std::vector<float>> dataset) const;
    TextureClassifier *train(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                             const uint pca = 0u);
    std::vector<std::string> estimate(const std::vector<std::vector<float>> features) const;

public:
    FeatureAnalyzer(const std::vector<std::string> *filterName,
                    const std::vector<std::string> *symFilterName);
    virtual ~FeatureAnalyzer();

    void analyze(const TextureDataset trainingDataset, const TextureDataset testDataset,
                 const uint pca, const char *trainPcaCsv, const char *testPcaCsv);
};

#endif
