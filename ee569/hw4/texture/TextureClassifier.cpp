/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <random>
#include <unordered_set>
#include <cmath>

#include "opencv2/core.hpp"
#include "opencv2/core/hal/hal.hpp"

#include "TextureClassifier.h"

#define __MAX_PCA_DATASET_SIZE (2000)

TextureClassifier::TextureClassifier() {}

TextureClassifier::~TextureClassifier() {}

TextureClassifier *TextureClassifier::train(const std::unordered_multimap<std::string, std::vector<float>>,
                                            const uint) {
    std::cerr << "Not Supported!\n";
    return this;
};

std::vector<std::string> TextureClassifier::estimate(const std::vector<std::vector<float>>) const {
    std::cerr << "Not Supported!\n";
    return std::vector<std::string>();
}

std::unordered_multimap<std::string, std::vector<float>>
    TextureClassifier::trainNorm(const std::unordered_multimap<std::string, std::vector<float>> dataset) {
    if (dataset.empty()) {
        return dataset;
    }

    std::vector<std::vector<float>> ppfFeatures;
    for (auto data : dataset) {
        ppfFeatures.push_back(data.second);
    }
    const auto datasetSize = ppfFeatures.size();
    const auto featureDim = ppfFeatures.front().size();
    std::vector<float> sum(featureDim, .0f);
    std::vector<float> mean, stdDev;
    for (auto pfFeature : ppfFeatures) {
        for (auto i = 0u; i < featureDim; ++i) {
            sum.at(i) += pfFeature.at(i);
        }
    }
    for (auto i = 0u; i < featureDim; ++i) {
        mean.push_back(sum.at(i) / datasetSize);
    }
    sum.clear();
    sum = std::vector<float>(featureDim, .0f);
    for (auto pfFeature : ppfFeatures) {
        for (auto i = 0u; i < featureDim; ++i) {
            auto d = pfFeature.at(i) - mean.at(i);
            sum.at(i) += d * d;
        }
    }
    for (auto i = 0u; i < featureDim; ++i) {
        stdDev.push_back(sqrt(sum.at(i) / datasetSize));
    }

    std::unordered_multimap<std::string, std::vector<float>> mDatasetReducedDim;
    for (auto data : dataset) {
        auto pfFeature = data.second;
        std::vector<float> pfFeatureNorm;
        for (auto i = 0u; i < featureDim; ++i) {
            pfFeatureNorm.push_back((pfFeature.at(i) - mean.at(i)) / stdDev.at(i));
        }
        mDatasetReducedDim.insert(std::make_pair(data.first, pfFeatureNorm));
    }

    this->m_pfNormMean = mean;
    this->m_pfNormStdDev = stdDev;
    mean.clear();
    stdDev.clear();

    return mDatasetReducedDim;
}

std::unordered_multimap<std::string, std::vector<float>>
    TextureClassifier::trainPca(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                                const uint dimension) {
    if (dataset.empty() ||
        !dimension|| dimension >= dataset.cbegin()->second.size()) {
        return dataset;
    }

    std::vector<std::vector<float>> ppfFeatures;
    for (auto data : dataset) {
        ppfFeatures.push_back(data.second);
    }
    const auto datasetSize = ppfFeatures.size();
    const auto featureDim = ppfFeatures.front().size();

    const auto sampleDatasetSize = std::min(datasetSize, (std::size_t) __MAX_PCA_DATASET_SIZE);
    const auto pcMat = new cv::Mat(sampleDatasetSize, featureDim, CV_32F);
    if (sampleDatasetSize < datasetSize) {
        std::random_device randDev;
        std::mt19937 generator(randDev());
        auto uniformDistribution = std::uniform_int_distribution<uint>(0, datasetSize - 1);
        std::unordered_set<uint> setIdx;
        while (setIdx.size() < sampleDatasetSize) {
            setIdx.insert(uniformDistribution(generator));
        }
        auto i = 0u;
        for (auto idx : setIdx) {
            for (auto j = 0u; j < featureDim; ++j) {
                pcMat->at<float>(i, j) = ppfFeatures.at(idx).at(j);
            }
            ++i;
        }
        for (auto i = 0u; i < datasetSize; ++i) {
            ppfFeatures.at(i).clear();
        }
    } else {
        for (auto i = 0u; i < datasetSize; ++i) {
            for (auto j = 0u; j < featureDim; ++j) {
                pcMat->at<float>(i, j) = ppfFeatures.at(i).at(j);
            }
            ppfFeatures.at(i).clear();
        }
    }
    ppfFeatures.clear();

    std::cout << "SVD start\n";
    const auto pcSVD = new cv::SVD(*pcMat, cv::SVD::Flags::FULL_UV);
    pcMat->release();
    delete pcMat;
    std::cout << "SVD done\n";

    this->m_ppfPcaVector.clear();
    const auto pcVtMat = pcSVD->vt;
    for (auto i = 0u; i < dimension; ++i) {
        std::vector<float> v;
        for (auto j = 0u; j < featureDim; ++j) {
            v.push_back(pcVtMat.at<float>(i, j));
        }
        this->m_ppfPcaVector.push_back(v);
    }

    std::unordered_multimap<std::string, std::vector<float>> mDatasetReducedDim;
    for (auto data : dataset) {
        auto pfFeature = data.second;
        std::vector<float> pfFeatureReducedDim;
        for (auto pcaVector : this->m_ppfPcaVector) {
            pfFeatureReducedDim.push_back(this->dot(pfFeature, pcaVector));
        }
        mDatasetReducedDim.insert(std::make_pair(data.first, pfFeatureReducedDim));
    }
    return mDatasetReducedDim;
}

std::vector<std::vector<float>> TextureClassifier::norm(const std::vector<std::vector<float>> features) const {
    if (this->m_pfNormMean.empty() || this->m_pfNormStdDev.empty()) {
        return features;
    }

    const auto featureDim = features.front().size();
    std::vector<std::vector<float>> ppfFeaturesNorm;
    for (auto pfFeature : features) {
        std::vector<float> pfFeatureNorm;
        for (auto i = 0u; i < featureDim; ++i) {
            pfFeatureNorm.push_back((pfFeature.at(i) - this->m_pfNormMean.at(i)) / this->m_pfNormStdDev.at(i));
        }
        ppfFeaturesNorm.push_back(pfFeatureNorm);
    }
    return ppfFeaturesNorm;
}

std::vector<std::vector<float>> TextureClassifier::pca(const std::vector<std::vector<float>> features) const {
    if (features.empty() || this->m_ppfPcaVector.empty()) {
        return features;
    }
    std::vector<std::vector<float>> pfFeaturesReducedDim;
    for (auto pfFeature : features) {
        std::vector<float> pfFeatureReducedDim;
        for (auto pcaVector : this->m_ppfPcaVector) {
            pfFeatureReducedDim.push_back(this->dot(pfFeature, pcaVector));
        }
        pfFeaturesReducedDim.push_back(pfFeatureReducedDim);
    }
    return pfFeaturesReducedDim;
}

float TextureClassifier::dot(const std::vector<float> v1,
                             const std::vector<float> v2) const {
    float fResult = .0f;
    if (v1.size() != v2.size()) {
        return fResult;
    }
    for (auto i = 0u; i < v1.size(); ++i) {
        fResult += v1.at(i) * v2.at(i);
    }
    return fResult;
}

float TextureClassifier::distanceSquare(const std::vector<float> v1,
                                        const std::vector<float> v2) const {
#ifdef __CV_HAL_OPT
    return cv::hal::normL2Sqr_(v1.data(), v2.data(), v1.size());
#else
    std::vector<float> v;
    for (auto i = 0u; i < v1.size(); ++i) {
        v.push_back(v1.at(i) - v2.at(i));
    }
    auto fResult = this->dot(v, v);
    v.clear();
    return fResult;
#endif
}
