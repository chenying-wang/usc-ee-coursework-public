/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <vector>

#include "FeatureAnalyzer.h"
#include "TextureClassifier.h"
#include "TextureDataset.h"

FeatureAnalyzer::FeatureAnalyzer(const std::vector<std::string> *filterName,
                                 const std::vector<std::string> *symFilterName) :
    m_ppsFilterName(filterName),
    m_ppsSymFilterName(symFilterName) {}

FeatureAnalyzer::~FeatureAnalyzer() {}

void FeatureAnalyzer::analyze(const TextureDataset trainingDataset, const TextureDataset testDataset,
                              const uint pca, const char *trainPcaCsv, const char *testPcaCsv) {
    if (!trainingDataset.size() || !trainingDataset.size() ||
        trainPcaCsv == nullptr || testPcaCsv == nullptr) {
        return;
    }

    auto mTrainingData = trainingDataset.packageTrainingData();
    this->analyzeFeatures(mTrainingData);

    this->trainPca(mTrainingData, pca);
    auto pcaTrainingData = this->pca(trainingDataset.getFeatures());
    std::vector<std::vector<std::string>> trainCsv;
    for (auto i = 0u; i < trainingDataset.size(); ++i) {
        auto line = std::vector<std::string>();
        for (auto pcaFeature : pcaTrainingData.at(i)) {
            line.push_back(std::to_string(pcaFeature));
        }
        line.push_back(trainingDataset.getLabel().at(i));
        trainCsv.push_back(line);
    }
    this->m_cCSVWriter.write(trainPcaCsv, trainCsv);

    auto pcaTestData = this->pca(testDataset.getFeatures());
    std::vector<std::vector<std::string>> testCsv;
    for (auto i = 0u; i < testDataset.size(); ++i) {
        auto line = std::vector<std::string>();
        for (auto pcaFeature : pcaTestData.at(i)) {
            line.push_back(std::to_string(pcaFeature));
        }
        line.push_back(testDataset.getLabel().at(i));
        testCsv.push_back(line);
    }
    this->m_cCSVWriter.write(testPcaCsv, testCsv);

    for (auto data : mTrainingData) {
        data.second.clear();
    }
    mTrainingData.clear();
}

void FeatureAnalyzer::analyzeFeatures(const std::unordered_multimap<std::string, std::vector<float>> dataset) const {
    if (dataset.empty()) {
        return;
    }

    auto datasetSize = dataset.size();
    auto featureDim = dataset.cbegin()->second.size();
    auto globalAverage = std::vector<float>(featureDim, .0f);
    auto classAverage = std::unordered_map<std::string, std::vector<float>>();
    auto intraclassVariance = std::vector<float>(featureDim, .0f);
    auto interclassVariance = std::vector<float>(featureDim, .0f);

    for (auto data : dataset) {
        for (auto i = 0u; i < featureDim; ++i) {
            globalAverage.at(i) += data.second.at(i);
        }
        if (!classAverage.count(data.first)) {
            classAverage.insert(std::make_pair(data.first, std::vector<float>(featureDim, .0f)));
        }
        for (auto i = 0u; i < featureDim; ++i) {
            classAverage.at(data.first).at(i) += data.second.at(i);
        }
    }

    for (auto i = 0u; i < featureDim; ++i) {
        globalAverage.at(i) /= datasetSize;
    }
    for (auto avg : classAverage) {
        auto count = dataset.count(avg.first);
        for (auto i = 0u; i < featureDim; ++i) {
            avg.second.at(i) /= count;
            auto diff = avg.second.at(i) - globalAverage.at(i);
            interclassVariance.at(i) += count * diff * diff;
        }
    }
    globalAverage.clear();
    for (auto data : dataset) {
        for (auto i = 0u; i < featureDim; ++i) {
            auto diff = data.second.at(i) - classAverage.at(data.first).at(i);
            intraclassVariance.at(i) += diff * diff;
        }
    }
    classAverage.clear();

    auto fMaxVarRatio = std::numeric_limits<float>::min(), fMinVarRatio = std::numeric_limits<float>::max();
    std::string strongestDiscriminantFeature, weakestDiscriminantFeature;
    for (auto i = 0u; i < featureDim; ++i) {
        auto featureName = this->m_ppsFilterName->at(i) + '/' + this->m_ppsSymFilterName->at(i);
        std::cout << featureName << ',' << intraclassVariance.at(i) << ',' << interclassVariance.at(i);
        auto fVarRatio = intraclassVariance.at(i) / interclassVariance.at(i);
        std::cout << ',' << fVarRatio << '\n';
        if (fVarRatio < fMinVarRatio) {
            strongestDiscriminantFeature = featureName;
            fMinVarRatio = fVarRatio;
        }
        if (fVarRatio > fMaxVarRatio) {
            weakestDiscriminantFeature= featureName;
            fMaxVarRatio = fVarRatio;
        }
    }
    std::cout << "Feature with Strongest Discriminant Power: " << strongestDiscriminantFeature << '\n';
    std::cout << "Feature with Weakest Discriminant Power: " << weakestDiscriminantFeature << '\n';
    intraclassVariance.clear();
    interclassVariance.clear();
}

TextureClassifier *FeatureAnalyzer::train(const std::unordered_multimap<std::string, std::vector<float>>,
                                          const uint) {
    std::cerr << "Not Supported!\n";
    return this;
}

std::vector<std::string> FeatureAnalyzer::estimate(const std::vector<std::vector<float>>) const {
    std::cerr << "Not Supported!\n";
    return std::vector<std::string>();
}
