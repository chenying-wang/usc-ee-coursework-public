/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>

#include "CvTextureClassifier.h"

CvTextureClassifier::CvTextureClassifier(const uint type) : m_uiType(type) {}

CvTextureClassifier::~CvTextureClassifier() {
    this->m_mLabelMapping.clear();
    this->m_mLabelReverseMapping.clear();
}

TextureClassifier *CvTextureClassifier::train(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                                              const uint pca) {
    if (dataset.empty()) {
        return this;
    }

    this->m_mLabelMapping.clear();
    this->m_mLabelReverseMapping.clear();

    auto i = 1u;
    for (auto data : dataset) {
        if (this->m_mLabelMapping.count(data.first)) {
            continue;
        }
        this->m_mLabelMapping.insert(std::make_pair(data.first, i));
        this->m_mLabelReverseMapping.insert(std::make_pair(i, data.first));
        ++i;
    }
    auto mDatasetReducedDim = this->trainPca(dataset, pca);

    const auto datasetSize = mDatasetReducedDim.size();
    const auto featureDim = mDatasetReducedDim.cbegin()->second.size();
    const auto pcFeaturesMat = new cv::Mat(datasetSize, featureDim, CV_32F);
    const auto pcLabelsMat = new cv::Mat(datasetSize, 1, CV_32S);
    i = 0u;
    for (auto dataReducedDim : mDatasetReducedDim) {
        for (auto j = 0u; j < featureDim; ++j) {
            pcFeaturesMat->at<float>(i, j) = dataReducedDim.second.at(j);
        }
        pcLabelsMat->at<int>(i, 0) = this->m_mLabelMapping.find(dataReducedDim.first)->second;
        ++i;
    }
    for (auto dataReducedDim : mDatasetReducedDim) {
        dataReducedDim.second.clear();
    }
    mDatasetReducedDim.clear();

    if (this->m_uiType == TEXTURE_OPENCV_MODEL_RF) {
        auto pcRTrees = cv::ml::RTrees::create();
        pcRTrees->train(*pcFeaturesMat, cv::ml::ROW_SAMPLE, *pcLabelsMat);
        this->m_pcCvModel = pcRTrees;
    } else if (this->m_uiType == TEXTURE_OPENCV_MODEL_SVM) {
        auto pcSvm = cv::ml::SVM::create();
        pcSvm->setType(cv::ml::SVM::C_SVC);
        pcSvm->setKernel(cv::ml::SVM::RBF);
        pcSvm->trainAuto(*pcFeaturesMat, cv::ml::ROW_SAMPLE, *pcLabelsMat);
        this->m_pcCvModel = pcSvm;
    } else {
        std::cerr << "Unknown model found!\n";
    }

    pcFeaturesMat->release();
    delete pcFeaturesMat;
    pcLabelsMat->release();
    delete pcLabelsMat;
    return this;
}

std::vector<std::string> CvTextureClassifier::estimate(const std::vector<std::vector<float>> features) const {
    std::vector<std::string> pphResult;
    if (!this->m_pcCvModel || features.empty()) {
        return pphResult;
    }

    auto ppfFeaturesReducedDim = this->pca(features);
    const auto datasetSize = ppfFeaturesReducedDim.size();
    const auto featureDim = ppfFeaturesReducedDim.front().size();
    const auto pcFeaturesMat = new cv::Mat(datasetSize, featureDim, CV_32F);
    for (auto i = 0u; i < datasetSize; ++i) {
        for (auto j = 0u; j < featureDim; ++j) {
            pcFeaturesMat->at<float>(i, j) = ppfFeaturesReducedDim.at(i).at(j);
        }
    }
    for (auto pfFeatureReducedDim : ppfFeaturesReducedDim) {
        pfFeatureReducedDim.clear();
    }
    ppfFeaturesReducedDim.clear();

    const auto pcResultMat = new cv::Mat(datasetSize, 1, CV_32F);
    this->m_pcCvModel->predict(*pcFeaturesMat, *pcResultMat);
    pcFeaturesMat->release();
    delete pcFeaturesMat;

    for (auto i = 0u; i < datasetSize; ++i) {
        auto result = (int) std::round(pcResultMat->at<float>(i, 0));
        if (!this->m_mLabelReverseMapping.count(result)) {
            pphResult.push_back(std::string("UNKNOWN"));
            continue;
        }
        pphResult.push_back(this->m_mLabelReverseMapping.find(result)->second);
    }
    pcResultMat->release();
    delete pcResultMat;
    return pphResult;
}
