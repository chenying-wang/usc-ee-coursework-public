/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <vector>
#include <unordered_map>

#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

#include "FeatureMatcher.h"

#define NUM_OF_SIFT_FEATURES (20)
#define NUM_OF_SIFT_BOW_FEATURES (500)
#define NUM_OF_SIFT_OCTAVES (3)
#define SIFT_CONTRAST_THRESHOLD (0.05)
#define SIFT_EDGE_THRESHOLD (12.0)
#define SIFT_SIGMA (2.0)

#define NUM_OF_BOW_BINS (8)

FeatureMatcher::FeatureMatcher() {
    this->m_pcFeatureDetector = cv::xfeatures2d::SIFT::create(NUM_OF_SIFT_FEATURES, NUM_OF_SIFT_OCTAVES,
        SIFT_CONTRAST_THRESHOLD, SIFT_EDGE_THRESHOLD, SIFT_SIGMA);
    this->m_pcBOWFeatureDetector = cv::xfeatures2d::SIFT::create(NUM_OF_SIFT_BOW_FEATURES, NUM_OF_SIFT_OCTAVES,
        SIFT_CONTRAST_THRESHOLD, SIFT_EDGE_THRESHOLD, SIFT_SIGMA);
    this->m_pcDescriptorMatcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_SL2);
}

FeatureMatcher::~FeatureMatcher() {}

Image *FeatureMatcher::getKeypoints(const cv::Mat *image) const {
    if (image == nullptr || image->empty()) {
        return nullptr;
    }

    auto keypoints = std::vector<cv::KeyPoint>();
    this->m_pcFeatureDetector->detect(*image, keypoints, cv::noArray());

    auto keypointsImage = this->drawKeypoints(image, keypoints);
    keypoints.clear();
    return keypointsImage;
}

Image *FeatureMatcher::getLargestScaleKeypoints(const cv::Mat *image) const {
    if (image == nullptr || image->empty()) {
        return nullptr;
    }

    auto keypoints = std::vector<cv::KeyPoint>();
    this->m_pcFeatureDetector->detect(*image, keypoints, cv::noArray());
    auto maxIdx = 0u;
    for (auto i = 0u; i < keypoints.size(); ++i) {
        if (keypoints.at(i).size > keypoints.at(maxIdx).size) {
            maxIdx = i;
        }
    }
    auto maxScaleKeypoint = std::vector<cv::KeyPoint>(1u, keypoints.at(maxIdx));
    auto largestScaleKeypointImage = this->drawKeypoints(image, maxScaleKeypoint);
    keypoints.clear();
    return largestScaleKeypointImage;
}

Image *FeatureMatcher::match(const cv::Mat *queryImage, const cv::Mat *trainImage) const {
    if (queryImage == nullptr || trainImage == nullptr ||
        queryImage->empty() || trainImage->empty()) {
        return nullptr;
    }

    auto queryKeypoints = std::vector<cv::KeyPoint>(), trainKeypoints = std::vector<cv::KeyPoint>();
    auto queryDescriptors = cv::Mat(), trainDescriptors = cv::Mat();
    this->m_pcFeatureDetector->detectAndCompute(*queryImage, cv::noArray(), queryKeypoints, queryDescriptors);
    this->m_pcFeatureDetector->detectAndCompute(*trainImage, cv::noArray(), trainKeypoints, trainDescriptors);

    auto matches = std::vector<cv::DMatch>();
    this->m_pcDescriptorMatcher->match(queryDescriptors, trainDescriptors, matches);
    queryDescriptors.release();
    trainDescriptors.release();

    Image *resultImage = this->drawMatches(queryImage, queryKeypoints, trainImage, trainKeypoints, matches);
    queryKeypoints.clear();
    trainKeypoints.clear();
    return resultImage;
}

Image *FeatureMatcher::matchLargestScaleKeypoint(const cv::Mat *queryImage, const cv::Mat *trainImage) const {
    if (queryImage == nullptr || trainImage == nullptr ||
        queryImage->empty() || trainImage->empty()) {
        return nullptr;
    }

    auto queryKeypoints = std::vector<cv::KeyPoint>(), trainKeypoints = std::vector<cv::KeyPoint>();
    auto queryDescriptors = cv::Mat(), trainDescriptors = cv::Mat();
    this->m_pcFeatureDetector->detect(*queryImage, queryKeypoints, cv::noArray());

    auto maxIdx = 0u;
    for (auto i = 0u; i < queryKeypoints.size(); ++i) {
        if (queryKeypoints.at(i).size > queryKeypoints.at(maxIdx).size) {
            maxIdx = i;
        }
    }
    auto maxScaleKeypoint = std::vector<cv::KeyPoint>(1u, queryKeypoints.at(maxIdx));
    this->m_pcFeatureDetector->compute(*queryImage, maxScaleKeypoint, queryDescriptors);
    this->m_pcFeatureDetector->detectAndCompute(*trainImage, cv::noArray(), trainKeypoints, trainDescriptors);

    auto matches = std::vector<cv::DMatch>();
    this->m_pcDescriptorMatcher->match(queryDescriptors, trainDescriptors, matches);
    queryDescriptors.release();
    trainDescriptors.release();

    auto resultImage = this->drawMatches(queryImage, maxScaleKeypoint, trainImage, trainKeypoints, matches);
    queryKeypoints.clear();
    trainKeypoints.clear();
    maxScaleKeypoint.clear();
    return resultImage;
}

Image *FeatureMatcher::bagOfWords(const cv::Mat *image, const char *outputCsvFilename) const {
    if (image->empty() || image->empty()) {
        return nullptr;
    }

    auto keypoints = std::vector<cv::KeyPoint>();
    auto descriptors = cv::Mat();
    this->m_pcBOWFeatureDetector->detectAndCompute(*image, cv::noArray(), keypoints, descriptors);

    auto labels = cv::Mat();
    cv::kmeans(descriptors, NUM_OF_BOW_BINS, labels,
        cv::TermCriteria(cv::TermCriteria::MAX_ITER, 2000u, 1e-7), 6,
        cv::KmeansFlags::KMEANS_PP_CENTERS);
    keypoints.clear();
    descriptors.release();

    std::unordered_map<int, uint> mCount;
    for (auto i = 0u; i < (uint) labels.rows; ++i) {
        auto label = labels.at<int>(i, 0);
        if (!mCount.count(label)) {
            mCount.insert(std::make_pair(label, 0u));
        }
        ++mCount.at(label);
    }
    labels.release();

    std::vector<std::vector<std::string>> csv;
    auto i = 0u;
    for (auto count : mCount) {
        csv.push_back(std::vector<std::string>({std::to_string(i++), std::to_string(count.second)}));
    }
    this->m_cCSVWriter.write(outputCsvFilename, csv);
    mCount.clear();
    csv.clear();

    return this->drawKeypoints(image, keypoints);
}

Image *FeatureMatcher::matchBagOfWords(const cv::Mat *image, const cv::Mat *referenceImages,
                                       const char *outputCsvFilename) const {
    if (image == nullptr || referenceImages == nullptr ||
        image->empty() || referenceImages->empty()) {
        return nullptr;
    }

    auto keypoints = std::vector<cv::KeyPoint>(), referenceKeypoints = std::vector<cv::KeyPoint>();
    auto descriptors = cv::Mat(), referenceDescriptors = cv::Mat();
    this->m_pcBOWFeatureDetector->detectAndCompute(*image, cv::noArray(), keypoints, descriptors);
    this->m_pcBOWFeatureDetector->detectAndCompute(*referenceImages, cv::noArray(), referenceKeypoints, referenceDescriptors);

    auto referenceLablesMat = cv::Mat(), centroids = cv::Mat();
    cv::kmeans(referenceDescriptors, NUM_OF_BOW_BINS, referenceLablesMat,
        cv::TermCriteria(cv::TermCriteria::MAX_ITER, 2000u, 1e-7), 6,
        cv::KmeansFlags::KMEANS_PP_CENTERS, centroids);

    auto labels = std::vector<uint>(NUM_OF_BOW_BINS, 0u);
    auto referenceLabels = std::vector<uint>(NUM_OF_BOW_BINS, 0u);
    for (auto i = 0u; i < (uint) descriptors.rows; ++i) {
        auto descriptor = descriptors.row(i);
        auto minDistance = std::numeric_limits<double>::max();
        auto label = 0u;
        for (auto j = 0u; j < (uint) NUM_OF_BOW_BINS; ++j) {
            auto distance = cv::norm(descriptor, centroids.row(j), cv::NormTypes::NORM_L2SQR);
            if (distance < minDistance) {
                minDistance = distance;
                label = j;
            }
        }
        ++labels.at(label);
    }
    for (auto i = 0u; i < (uint) referenceLablesMat.rows; ++i) {
        ++referenceLabels.at(referenceLablesMat.at<int>(i, 0));
    }
    referenceLablesMat.release();

    std::vector<std::vector<std::string>> csv;
    for (auto i = 0u; i < (uint) NUM_OF_BOW_BINS; ++i) {
        csv.push_back(std::vector<std::string>({
            std::to_string(i), std::to_string(labels.at(i)), std::to_string(referenceLabels.at(i))
        }));
    }
    this->m_cCSVWriter.write(outputCsvFilename, csv);
    labels.clear();
    csv.clear();

    return this->drawKeypoints(image, keypoints);
}

Image *FeatureMatcher::drawMatches(const cv::Mat *image1, const std::vector<cv::KeyPoint> keypoints1,
                                   const cv::Mat *image2, const std::vector<cv::KeyPoint> keypoints2,
                                   const std::vector<cv::DMatch> matches) const {
    if (matches.empty()) {
        return nullptr;
    }

    const auto pcMathesImageMat  = new cv::Mat();
    cv::drawMatches(*image1, keypoints1, *image2, keypoints2, matches, *pcMathesImageMat,
        cv::Scalar::all(-1), cv::Scalar::all(-1), std::vector<char>(),
        cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    const auto pcMatchesImage = this->m_cImageCvAdapter.fromCvMat(pcMathesImageMat);
    pcMathesImageMat->release();
    delete pcMathesImageMat;

    return pcMatchesImage;
}

Image *FeatureMatcher::drawKeypoints(const cv::Mat *image, const std::vector<cv::KeyPoint> keypoints) const {
    const auto pcKeypointsImageMat  = new cv::Mat();
    cv::drawKeypoints(*image, keypoints, *pcKeypointsImageMat,
        cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    const auto pcKeypointsImage = this->m_cImageCvAdapter.fromCvMat(pcKeypointsImageMat);
    pcKeypointsImageMat->release();
    delete pcKeypointsImageMat;

    return pcKeypointsImage;
}
