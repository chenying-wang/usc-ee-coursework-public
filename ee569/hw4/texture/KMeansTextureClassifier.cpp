/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <unordered_set>
#include <future>
#include <limits>
#include <random>

#include "opencv2/core.hpp"

#include "KMeansTextureClassifier.h"

#define __MAX_ITER (25u)
#define __PAGE_SIZE (1024u)

KMeansTextureClassifier::KMeansTextureClassifier(const uint K, const bool norm) :
    m_uiK(K),
    m_bNorm(norm) {}

KMeansTextureClassifier::~KMeansTextureClassifier() {
    for (auto pfClusterCentroid : this->m_ppfClusterCentroids) {
        pfClusterCentroid.clear();
    }
    this->m_ppfClusterCentroids.clear();
    this->m_psLables.clear();
}

TextureClassifier *KMeansTextureClassifier::train(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                                                  const uint pca) {
    if (dataset.empty()) {
        return this;
    }

    auto mDatasetNorm = std::unordered_multimap<std::string, std::vector<float>>();
    if (this->m_bNorm && pca) {
        mDatasetNorm = this->trainNorm(dataset);
    } else {
        mDatasetNorm = dataset;
    }
    auto mDatasetNormReducedDim = this->trainPca(mDatasetNorm, pca);
    if (this->m_bNorm && pca) {
        for (auto dataNorm : mDatasetNorm) {
            dataNorm.second.clear();
        }
        mDatasetNorm.clear();
    }

    std::vector<std::vector<float>> ppfFeatures;
    std::unordered_set<std::string> setsLabels;
    for (auto data : mDatasetNormReducedDim) {
        ppfFeatures.push_back(data.second);
        if (setsLabels.count(data.first)) {
            continue;
        }
        setsLabels.insert(data.first);
    }

    const auto uiSize = ppfFeatures.size();
    const auto uiDim = ppfFeatures.front().size();
    if (!this->m_uiK) {
        this->m_uiK = setsLabels.size();
    }

    for (auto dataNormReducedDim : mDatasetNormReducedDim) {
        dataNormReducedDim.second.clear();
    }
    mDatasetNormReducedDim.clear();
    setsLabels.clear();

    auto ppfCentorids = this->init(ppfFeatures);
    auto ppfSum = std::vector<std::vector<float>>();
    auto puiCount = std::vector<uint>();
    std::vector<uint> puiCluster(uiSize, 0u), puiPrevCluster;

    const auto task = [this](std::vector<float> feature, std::vector<std::vector<float>> meansNorm) {
        auto fMinDistanceSquare = std::numeric_limits<float>::max();
        auto cluster = 0u;
        for (auto i = 0u; i < this->m_uiK; ++i) {
            auto fDistanceSquare = this->distanceSquare(feature, meansNorm.at(i));
            if (fDistanceSquare < fMinDistanceSquare) {
                cluster = i;
                fMinDistanceSquare = fDistanceSquare;
            }
        }
        return cluster;
    };

    auto iter = 0u;
    while (iter < __MAX_ITER) {
        std::cout << "Iteration: #" << iter << '\n';
        std::vector<std::future<uint>> pcFuture;
        for (auto i = 0u, j = 0u; j < uiSize; ++i) {
            pcFuture.push_back(std::async(task, ppfFeatures.at(i), ppfCentorids));
            if (pcFuture.size() < __PAGE_SIZE && i < uiSize - 1) {
                continue;
            }
            for (auto k = 0u; k < pcFuture.size(); ++k) {
                puiCluster[j + k] = pcFuture.at(k).get();
            }
            j += pcFuture.size();
            pcFuture.clear();
        }

        if (puiPrevCluster.size()) {
            auto bIdenticalToPrev = true;
            for (auto i = 0u; i < puiCluster.size(); ++i) {
                bIdenticalToPrev &= puiCluster.at(i) == puiPrevCluster.at(i);
                if (!bIdenticalToPrev) {
                    continue;
                }
            }
            if (bIdenticalToPrev) {
                break;
            }
        }
        puiPrevCluster = puiCluster;

        for (auto i = 0u; i < this->m_uiK; ++i) {
            ppfSum.push_back(std::vector<float>(uiDim, .0f));
        }
        puiCount = std::vector<uint>(this->m_uiK, 0u);

        for (auto i = 0u; i < uiSize; ++i) {
            for (auto j = 0u; j < uiDim; ++j) {
                ppfSum.at(puiCluster.at(i)).at(j) += ppfFeatures.at(i).at(j);
            }
            ++puiCount.at(puiCluster.at(i));
        }
        for (auto i = 0u; i < this->m_uiK; ++i) {
            if (!puiCount.at(i)) {
                ppfSum.at(i).clear();
                continue;
            }
            for (auto j = 0u; j < uiDim; ++j) {
                ppfCentorids.at(i).at(j) = ppfSum.at(i).at(j) / puiCount.at(i);
            }
            ppfSum.at(i).clear();
        }
        ppfSum.clear();
        puiCount.clear();
        ++iter;
    }

    puiCluster.clear();
    puiPrevCluster.clear();
    for (auto pfFeature : ppfFeatures) {
        pfFeature.clear();
    }
    ppfFeatures.clear();

    this->m_ppfClusterCentroids = ppfCentorids;
    for (auto pfMeansNorm: ppfCentorids) {
        pfMeansNorm.clear();
    }
    ppfCentorids.clear();
    return this;
}

std::vector<std::string> KMeansTextureClassifier::estimate(const std::vector<std::vector<float>> features) const {
    std::vector<std::string> puiCluster;
    if (features.empty()) {
        return puiCluster;
    }

    auto ppfFeaturesNorm = this->norm(features);
    auto ppfFeaturesNormReducedDim = this->pca(ppfFeaturesNorm);
    for (auto pfFeatureNorm : ppfFeaturesNorm) {
        pfFeatureNorm.clear();
    }
    ppfFeaturesNorm.clear();

    const auto k = this->m_ppfClusterCentroids.size();
    for (auto pfFeatureNormReducedDim : ppfFeaturesNormReducedDim) {
        auto fMinDistanceSquare = std::numeric_limits<float>::max();
        auto cluster = 0u;
        for (auto i = 0u; i < k; ++i) {
            auto fDistanceSquare = this->distanceSquare(pfFeatureNormReducedDim, this->m_ppfClusterCentroids.at(i));
            if (fDistanceSquare < fMinDistanceSquare) {
                cluster = i;
                fMinDistanceSquare = fDistanceSquare;
            }
        }
        pfFeatureNormReducedDim.clear();
        puiCluster.push_back(std::to_string(1 + cluster));
    }
    ppfFeaturesNormReducedDim.clear();
    return puiCluster;
}

std::vector<std::vector<float>> KMeansTextureClassifier::init(const std::vector<std::vector<float>> features) const {
    const auto dataSize = features.size();
    if (features.size() <= this->m_uiK) {
        return features;
    }

    std::random_device randDev;
    std::mt19937 generator(randDev());
    auto uniformDistribution = std::uniform_real_distribution<float>(0, dataSize);

    std::unordered_set<uint> setIdx;
    setIdx.insert(std::floor(uniformDistribution(generator)));
    std::vector<float> pfDistanceSquare;
    while (setIdx.size() < this->m_uiK) {
        pfDistanceSquare.push_back(.0f);
        for (auto i = 0u; i < dataSize; ++i) {
            if (setIdx.count(i)) {
                pfDistanceSquare.push_back(pfDistanceSquare.back());
                continue;
            }
            auto fMinDistanceSquare = std::numeric_limits<float>::max();
            for (auto idx : setIdx) {
                fMinDistanceSquare = std::min(this->distanceSquare(features.at(i), features.at(idx)), fMinDistanceSquare);
            }
            pfDistanceSquare.push_back(pfDistanceSquare.back() + fMinDistanceSquare);
        }
        uniformDistribution = std::uniform_real_distribution<float>(0, pfDistanceSquare.back());
        auto rand = uniformDistribution(generator);
        for (auto i = 0u; i < dataSize; ++i) {
            if (rand >= pfDistanceSquare.at(i) && rand < pfDistanceSquare.at(i + 1)) {
                setIdx.insert(i);
                break;
            }
        }
        pfDistanceSquare.clear();
    }
    std::vector<std::vector<float>> ppfMeans;
    for (auto idx : setIdx) {
        ppfMeans.push_back(features.at(idx));
    }
    setIdx.clear();
    return ppfMeans;
}
