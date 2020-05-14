/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <unordered_map>
#include <functional>
#include <future>
#include <algorithm>
#include <cmath>

#include "TextureProcess.h"
#include "TextureClassifier.h"
#include "TextureDataset.h"
#include "FeatureAnalyzer.h"
#include "common/CSVReader.h"
#include "common/Filter.h"
#include "common/ImageIO.h"

#define __LAWS_FILTER_SIZE (5u)

TextureProcess::TextureProcess() {}

TextureProcess::~TextureProcess() {
    if (this->m_pppcFilter != nullptr) {
        for (auto filter : *this->m_pppcFilter) {
            delete filter;
        }
        this->m_pppcFilter->clear();
        delete this->m_pppcFilter;
    }
    if (this->m_psFilterName != nullptr) {
        this->m_psFilterName->clear();
        delete this->m_psFilterName;
    }
    if (this->m_pppcSymFilter != nullptr) {
        for (auto filter : *this->m_pppcSymFilter) {
            delete filter;
        }
        this->m_pppcSymFilter->clear();
        delete this->m_pppcSymFilter;
    }
    if (this->m_psSymFilterName != nullptr) {
        this->m_psSymFilterName->clear();
        delete this->m_psSymFilterName;
    }
}

TextureProcess *TextureProcess::load(const char *kernelFilename) {
    if (kernelFilename == nullptr) {
        return this;
    }

    if (this->m_pppcFilter != nullptr) {
        for (auto filter : *this->m_pppcFilter) {
            delete filter;
        }
        this->m_pppcFilter->clear();
    }
    if (this->m_psFilterName != nullptr) {
        this->m_psFilterName->clear();
    }
    if (this->m_pppcSymFilter != nullptr) {
        for (auto filter : *this->m_pppcSymFilter) {
            delete filter;
        }
        this->m_pppcSymFilter->clear();
    }
    if (this->m_psSymFilterName != nullptr) {
        this->m_psSymFilterName->clear();
    }

    auto ppphCsv = this->m_cCsvReader.read(kernelFilename);
    if (ppphCsv.empty()) {
        return this;
    }

    const auto lawsFilterSize = ppphCsv.front().size() - 1;

    const auto psKernelName = new std::vector<std::string>();
    const auto ppiKernels = new std::vector<const int *>();
    for (auto kernel : ppphCsv) {
        if (kernel.size() != lawsFilterSize + 1) {
            kernel.clear();
            continue;
        }

        psKernelName->push_back(kernel.front());
        const auto piKernel = new int[lawsFilterSize];
        for (auto i = 0u; i < lawsFilterSize; ++i) {
            piKernel[i] = atoi(kernel.at(1u + i).c_str());
        }
        ppiKernels->push_back(piKernel);
        kernel.clear();
    }
    ppphCsv.clear();
    if (ppiKernels->empty()) {
        return this;
    }

    const auto kernelNum = ppiKernels->size();
    const auto pppcFilter = new std::vector<const Filter *>();
    const auto psFilterName = new std::vector<std::string>();
    const auto pppcSymFilter = new std::vector<const Filter *>();
    const auto psSymFilterName = new std::vector<std::string>();
    for (auto i = 0u; i < kernelNum; ++i) {
        auto piKernalX = ppiKernels->at(i);
        for (auto j = i; j < kernelNum; ++j) {
            auto piKernalY = ppiKernels->at(j);
            auto ppfFilter = new const float *[lawsFilterSize];
            auto ppfSymFilter = new const float *[lawsFilterSize];
            for (auto k = 0u; k < lawsFilterSize; ++k) {
                auto ppfFilterArray = new float[lawsFilterSize];
                auto ppfSymFilterArray = new float[lawsFilterSize];
                for (auto l = 0u; l < lawsFilterSize; ++l) {
                    ppfFilterArray[l] = piKernalX[k] * piKernalY[l];
                    ppfSymFilterArray[l] = piKernalY[k] * piKernalX[l];
                }
                ppfFilter[k] = ppfFilterArray;
                ppfSymFilter[k] = ppfSymFilterArray;
            }
            pppcFilter->push_back(new const Filter(ppfFilter, lawsFilterSize, lawsFilterSize));
            psFilterName->push_back(psKernelName->at(i) + psKernelName->at(j));
            pppcSymFilter->push_back(new const Filter(ppfSymFilter, lawsFilterSize, lawsFilterSize));
            psSymFilterName->push_back(psKernelName->at(j) + psKernelName->at(i));
        }
    }

    this->m_pppcFilter = pppcFilter;
    this->m_psFilterName = psFilterName;
    this->m_pppcSymFilter = pppcSymFilter;
    this->m_psSymFilterName = psSymFilterName;
    return this;
}

const TextureProcess *TextureProcess::train(TextureClassifier *textureClassifier, const uint pca,
                                            const char *trainingDatasetFilename,
                                            const uint width, const uint height) const {
    if (textureClassifier == nullptr || trainingDatasetFilename == nullptr) {
        return this;
    }

    std::cout << "Train on: " << trainingDatasetFilename << '\n';
    auto dataset = this->loadDataset(trainingDatasetFilename, width, height);
    auto mTrainingData = dataset.packageTrainingData();
    textureClassifier->train(mTrainingData, pca);
    for (auto trainingData : mTrainingData) {
        trainingData.second.clear();
    }
    mTrainingData.clear();
    dataset.clear();
    return this;
}

const TextureProcess *TextureProcess::test(const TextureClassifier *textureClassifier,
                                           const char *testDatasetFilename,
                                           const uint width, const uint height) const {
    if (textureClassifier == nullptr || testDatasetFilename == nullptr) {
        return this;
    }

    std::cout << "Test on: " << testDatasetFilename << '\n';
    auto dataset = this->loadDataset(testDatasetFilename, width, height);
    auto pphTestResult = textureClassifier->estimate(dataset.getFeatures());

    for (auto i = 0u; i < dataset.size(); ++i) {
        std::cout << std::get<0>(dataset.at(i)) << ',' << std::get<2>(dataset.at(i)) << ':' << pphTestResult.at(i) << '\n';
    }
    auto uiCorrectCount = this->maxAccuracy(pphTestResult, dataset.getLabel());
    std::cout << "Accuracy: " << uiCorrectCount << '/' << dataset.size();
    std::cout << " = " << 1.f * uiCorrectCount / dataset.size() << '\n';
    dataset.clear();
    return this;
}

const TextureProcess *TextureProcess::analyze(const char *trainingDatasetFilename, const char *testDatasetFilename,
                                              const uint pca, const uint width, const uint height,
                                              const char *trainPcaCsv, const char *testPcaCsv) const {
    if (trainingDatasetFilename == nullptr || testDatasetFilename == nullptr ||
        trainPcaCsv == nullptr || testPcaCsv == nullptr) {
        return this;
    }

    auto trainingData = this->loadDataset(trainingDatasetFilename, width, height);
    auto testData = this->loadDataset(testDatasetFilename, width, height);

    auto featureAnalyzer = FeatureAnalyzer(this->m_psFilterName, this->m_psSymFilterName);
    featureAnalyzer.analyze(trainingData, testData, pca, trainPcaCsv, testPcaCsv);

    trainingData.clear();
    testData.clear();
    return this;
}

Image *TextureProcess::cluster(TextureClassifier *textureClassifier, const uint pca,
                               const Image *image, const uint k, const uint windowSize,
                               const bool optWindows) const {
    if (textureClassifier == nullptr) {
        return nullptr;
    }

    auto pmTrainingData = new std::unordered_multimap<std::string, std::vector<float>>();
    auto ppfFeatures = this->extractFeaturesByPixel(image, windowSize);
    auto weight = std::vector<float>();
    if (optWindows) {
        weight = std::vector<float>({1, 0.25, 0.4, 0.25});
    } else {
        weight = std::vector<float>({1, 0, 0, 0});
    }
    auto ppfFeaturesNorm = this->norm(ppfFeatures, image->getWidth(), image->getHeight(), weight);

    image->rasterScan([&pmTrainingData, ppfFeaturesNorm, &k](const uint i, const uint j) {
        pmTrainingData->insert(std::make_pair(std::to_string((i + j) % k), ppfFeaturesNorm[i][j]));
    });

    textureClassifier->train(*pmTrainingData, pca);
    for (auto trainingData : *pmTrainingData) {
        trainingData.second.clear();
    }
    pmTrainingData->clear();
    delete pmTrainingData;

    const PIXEL_TYPE step = MAX_PIXEL_VALUE / (k - 1);
    auto resultMap = image->concurrentMap([textureClassifier, ppfFeaturesNorm, &step](const uint i, const uint j) {
        std::vector<std::vector<float>> input;
        input.push_back(ppfFeaturesNorm[i][j]);
        PIXEL_TYPE result = step * (std::stoi(textureClassifier->estimate(input).front()) - 1);
        return new PIXEL_TYPE[NUM_OF_CHANNELS_GRAY]{result};
    });

    auto preProcessedResultMap = resultMap;
    for (auto i = 0u; i < 5u; ++i) {
        auto postProcessedMap = preProcessedResultMap->concurrentMap([preProcessedResultMap](const uint i, const uint j) {
            auto current = preProcessedResultMap->getImage()[i][j][CHANNEL_GRAY];
            auto top = preProcessedResultMap->getPixel(i - 1, j)[CHANNEL_GRAY];
            auto bottom = preProcessedResultMap->getPixel(i + 1, j)[CHANNEL_GRAY];
            auto left = preProcessedResultMap->getPixel(i, j - 1)[CHANNEL_GRAY];
            auto right = preProcessedResultMap->getPixel(i, j + 1)[CHANNEL_GRAY];
            if (current != top && top == bottom) {
                return new PIXEL_TYPE[NUM_OF_CHANNELS_GRAY]{top};
            } else if (current != left && left == right) {
                return new PIXEL_TYPE[NUM_OF_CHANNELS_GRAY]{left};
            }
            return new PIXEL_TYPE[NUM_OF_CHANNELS_GRAY]{current};
        });
        delete preProcessedResultMap;
        preProcessedResultMap = postProcessedMap;
    }
    return preProcessedResultMap;
}

TextureDataset TextureProcess::loadDataset(const char *datasetFilename, const uint width, const uint height) const {
    auto phTestTexture = std::vector<std::string>();
    auto ppfTestFeatures = std::vector<std::vector<float>>();
    auto phTestLabel = std::vector<std::string>();

    TextureDataset dataset;
    if (datasetFilename == nullptr) {
        return dataset;
    }

    auto ppphCsv = this->m_cCsvReader.read(datasetFilename);
    if (ppphCsv.empty()) {
        return dataset;
    }

    const auto task = [this, &dataset, &width, &height] (std::string texture, std::string label) {
        const auto pcImage = new Image(width, height, NUM_OF_CHANNELS_GRAY, sizeof(PIXEL_TYPE), PaddingType::EVEN_REFLECT);
        this->m_cImageIO.readRawImage(pcImage, texture.c_str());
        const auto pfFeatures = this->extractFeatures(pcImage);
        dataset.insert(texture, pfFeatures, label);
        pcImage->destroy();
        delete pcImage;
    };

    std::vector<std::future<void>> pcFuture;
    for (auto phData : ppphCsv) {
        if (phData.size() != 2u) {
            continue;
        }
        pcFuture.push_back(std::async(task, phData.at(0), phData.at(1)));
        phData.clear();
    }
    ppphCsv.clear();
    for (auto i = 0u; i < pcFuture.size(); ++i) {
        pcFuture.at(i).get();
    }
    pcFuture.clear();

    return dataset;
}

std::vector<float> TextureProcess::extractFeatures(const Image *image) const {
    const auto pppuhImage = image->getImage();
    auto uiSum = 0u;
    image->rasterScan([pppuhImage, &uiSum](const uint i, const uint j) {
        uiSum += pppuhImage[i][j][CHANNEL_GRAY];
    });
    uiSum /= (image->getWidth() * image->getHeight());
    image->concurrentScan([pppuhImage, &uiSum](const uint i, const uint j) {
        if (pppuhImage[i][j][CHANNEL_GRAY] <= uiSum) {
            pppuhImage[i][j][CHANNEL_GRAY] = 0u;
            return;
        }
        pppuhImage[i][j][CHANNEL_GRAY] -= uiSum;
    });

    std::vector<float> pfFeatures;
    const auto filterNum = this->m_pppcFilter->size();
    for (auto i = 0u; i < filterNum; ++i) {
        auto pcFilter = this->m_pppcFilter->at(i);
        auto pcSymFilter = this->m_pppcSymFilter->at(i);
        auto fFeatureSum = .0f;
        image->rasterScan([pcFilter, pcSymFilter, image, &fFeatureSum](const uint i, const uint j) {
            fFeatureSum += (
                std::abs(pcFilter->filterImageWithoutNorm(image, i, j, CHANNEL_GRAY)) +
                std::abs(pcSymFilter->filterImageWithoutNorm(image, i, j, CHANNEL_GRAY))) / 2;
        });
        pfFeatures.push_back(fFeatureSum / image->getWidth() / image->getHeight());
    }
    return pfFeatures;
}

std::vector<float> **TextureProcess::extractFeaturesByPixel(const Image *image, const uint windowSize) const {
    const auto uiWidth = image->getWidth(), uiHeight = image->getHeight();
    const auto pppuhImage = image->getImage();
    auto uiMean = 0u;
    image->rasterScan([pppuhImage, &uiMean](const uint i, const uint j) {
        uiMean += pppuhImage[i][j][CHANNEL_GRAY];
    });
    uiMean /= (uiWidth * uiHeight);
    image->concurrentScan([pppuhImage, &uiMean](const uint i, const uint j) {
        if (pppuhImage[i][j][CHANNEL_GRAY] <= uiMean) {
            pppuhImage[i][j][CHANNEL_GRAY] = 0u;
            return;
        }
        pppuhImage[i][j][CHANNEL_GRAY] = pppuhImage[i][j][CHANNEL_GRAY] - uiMean;
    });

    const auto pppcFilters = this->m_pppcFilter;
    const auto pppcSymFilters = this->m_pppcSymFilter;
    auto ppfFeatures = new std::vector<float>*[uiHeight];
    for (auto i = 0u; i < uiHeight; ++i) {
        ppfFeatures[i] = new std::vector<float>[uiWidth];
    }

    const auto filterNum = pppcFilters->size();
    image->concurrentScan([pppcFilters, pppcSymFilters, image, ppfFeatures, &filterNum](const uint i, const uint j) {
        for (auto k = 1u; k < filterNum; ++k) {
            auto pcFilter = pppcFilters->at(k);
            auto pcSymFilter = pppcFilters->at(k);
            ppfFeatures[i][j].push_back(
                std::abs(pcFilter->filterImageWithoutNorm(image, i, j, CHANNEL_GRAY)) +
                std::abs(pcSymFilter->filterImageWithoutNorm(image, i, j, CHANNEL_GRAY)));
        }
    });

    auto ppfPoolingFeatures = new std::vector<float>*[uiHeight];
    for (auto i = 0u; i < image->getHeight(); ++i) {
        ppfPoolingFeatures[i] = new std::vector<float>[uiWidth];
    }

    image->concurrentScan([ppfFeatures, ppfPoolingFeatures, &windowSize, &uiWidth, &uiHeight]
    (const uint i, const uint j) {
        const auto halfWindowSize = windowSize >> 1;
        const auto filterNum = ppfFeatures[i][j].size();
        auto pfFeatureSum = std::vector<float>(filterNum, .0f);

        auto pfDistance = std::vector<float>();
        auto pfDistanceSorted = std::vector<float>();
        auto idx = std::vector<std::pair<uint, uint>>();
        auto distance = .0f;
        for (auto x = 0u; x < windowSize; ++x) {
            auto _x = abs((int) i - halfWindowSize + x) % (uiHeight - 1);
            if (_x > uiHeight - 1) {
                _x = 2 * uiHeight - 2 - _x;
            }
            for (auto y = 0u; y < windowSize; ++y) {
                auto _y = abs((int) j - halfWindowSize + y) % (2 * uiWidth - 1);
                if (_y > uiWidth - 1) {
                    _y = 2 * uiWidth - 2 - _y;
                }

                distance = .0f;
                for (auto k = 0u; k < filterNum; ++k) {
                    pfFeatureSum.at(k) = ppfFeatures[_x][_y].at(k) + pfFeatureSum.at(k);
                    distance += ppfFeatures[_x][_y].at(k) * ppfFeatures[_x][_y].at(k);
                }
                pfDistance.push_back(distance);
                pfDistanceSorted.push_back(distance);
                idx.push_back(std::make_pair(_x, _y));
            }
        }
        std::sort(pfDistanceSorted.begin(), pfDistanceSorted.end());
        auto maxIdx = std::find(pfDistance.begin(), pfDistance.end(),
            pfDistanceSorted.back()) - pfDistance.begin();
        auto medianIdx = std::find(pfDistance.begin(), pfDistance.end(),
            pfDistanceSorted.at(pfDistanceSorted.size() / 2)) - pfDistance.begin();
        auto minIdx = std::find(pfDistance.begin(), pfDistance.end(),
            pfDistanceSorted.front()) - pfDistance.begin();

        auto pfFeatureMax = std::vector<float>(filterNum, .0f);
        auto pfFeatureMedian = std::vector<float>(filterNum, .0f);
        auto pfFeatureMin = std::vector<float>(filterNum, .0f);
        for (auto k = 0u; k < filterNum; ++k) {
            pfFeatureMax.at(k) = ppfFeatures[idx.at(maxIdx).first][idx.at(maxIdx).second].at(k);
        }
        for (auto k = 0u; k < filterNum; ++k) {
            pfFeatureMedian.at(k) = ppfFeatures[idx.at(medianIdx).first][idx.at(medianIdx).second].at(k);
        }
        for (auto k = 0u; k < filterNum; ++k) {
            pfFeatureMin.at(k) = ppfFeatures[idx.at(minIdx).first][idx.at(minIdx).second].at(k);
        }

        for (auto fFeature : pfFeatureSum) {
            ppfPoolingFeatures[i][j].push_back(fFeature);
        }
        for (auto fFeature : pfFeatureMax) {
            ppfPoolingFeatures[i][j].push_back(fFeature);
        }
        for (auto fFeature : pfFeatureMedian) {
            ppfPoolingFeatures[i][j].push_back(fFeature);
        }
        for (auto fFeature : pfFeatureMin) {
            ppfPoolingFeatures[i][j].push_back(fFeature);
        }
    });
    return ppfPoolingFeatures;
}

std::vector<float> **TextureProcess::norm(std::vector<float> **features,
                                          const uint width, const uint height,
                                          const std::vector<float> weight) const {
    if (features == nullptr) {
        return nullptr;
    }

    const auto datasetSize = width * height;
    const auto featureDim = features[0][0].size();
    auto pfFeaturesNormMean = std::vector<float>(featureDim, .0f);
    for (auto i = 0u; i < height; ++i) {
        for (auto j = 0u; j < width; ++j) {
            auto pfFeature = features[i][j];
            for (auto k = 0u; k < featureDim; ++k) {
                pfFeaturesNormMean.at(k) += pfFeature.at(k);
            }
        }
    }
    for (auto i = 0u; i < featureDim; ++i) {
        pfFeaturesNormMean.at(i) /= datasetSize;
    }

    auto pfFeaturesNormStdDev = std::vector<float>(featureDim, .0f);
    for (auto i = 0u; i < height; ++i) {
        for (auto j = 0u; j < width; ++j) {
            auto pfFeature = features[i][j];
            for (auto k = 0u; k < featureDim; ++k) {
                auto diff = pfFeature.at(k) - pfFeaturesNormMean.at(k);
                pfFeaturesNormStdDev.at(k) += diff * diff;
            }
        }
    }
    for (auto i = 0u; i < featureDim; ++i) {
        pfFeaturesNormStdDev.at(i) = sqrt(pfFeaturesNormStdDev.at(i) / datasetSize);
    }

    auto weightSize = weight.size();
    auto ppfFeaturesNorm = new std::vector<float> *[height];
    for (auto i = 0u; i < height; ++i) {
        ppfFeaturesNorm[i] = new std::vector<float>[width];
        for (auto j = 0u; j < width; ++j) {
            auto pfFeature = features[i][j];
            for (auto k = 0u; k < featureDim; ++k) {
                auto w = sqrt(weight.at(k * weightSize / featureDim));
                if (w < std::numeric_limits<double>::epsilon()) {
                    continue;
                }
                ppfFeaturesNorm[i][j].push_back(w *
                    (pfFeature.at(k) - pfFeaturesNormMean.at(k)) / pfFeaturesNormStdDev.at(k));
            }
        }
    }
    return ppfFeaturesNorm;
}

uint TextureProcess::maxAccuracy(const std::vector<std::string> estimations, const std::vector<std::string> labels,
                                 std::vector<std::string> psEstimation, std::vector<std::string> psLabel,
                                 std::unordered_set<uint> usedLabel) const {
    if (psLabel.empty()) {
        psEstimation = std::vector<std::string>();
        psLabel = std::vector<std::string>();
        usedLabel = std::unordered_set<uint>();
        std::unordered_set<std::string> set;
        for (auto sEstimation : estimations) {
            if (set.count(sEstimation)) {
                continue;
            }
            set.insert(sEstimation);
            psEstimation.push_back(sEstimation);
        }
        set.clear();
        for (auto sLabel : labels) {
            if (set.count(sLabel)) {
                continue;
            }
            set.insert(sLabel);
            psLabel.push_back(sLabel);
        }
        set.clear();
    }

    if (psEstimation.empty() || psLabel.size() == usedLabel.size()) {
        return 0u;
    }

    auto estimation = psEstimation.back();
    psEstimation.pop_back();
    auto maxAccuracy = 0u;

    for (auto i = 0u; i < psLabel.size(); ++i) {
        if (usedLabel.count(i)) {
            continue;
        }
        auto accuracy = 0u;
        auto label = psLabel.at(i);
        for (auto j = 0u; j < estimations.size(); ++j) {
            if (!estimations.at(j).compare(estimation) && !labels.at(j).compare(label)) {
                ++accuracy;
            }
        }
        usedLabel.insert(i);
        accuracy += this->maxAccuracy(estimations, labels, psEstimation, psLabel, usedLabel);
        usedLabel.erase(i);
        maxAccuracy = std::max(accuracy, maxAccuracy);
    }
    return maxAccuracy;
}
