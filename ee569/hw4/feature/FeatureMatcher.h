/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __FEATURE_MATCHER
#define __FEATURE_MATCHER

#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"

#include "ImageCvAdapter.h"
#include "common/CSVWriter.h"

class FeatureMatcher {
private:
    cv::Ptr<cv::Feature2D> m_pcFeatureDetector = nullptr;
    cv::Ptr<cv::Feature2D> m_pcBOWFeatureDetector = nullptr;
    cv::Ptr<cv::DescriptorMatcher> m_pcDescriptorMatcher = nullptr;
    ImageCvAdapter m_cImageCvAdapter;
    CSVWriter m_cCSVWriter;

    Image *drawMatches(const cv::Mat *image1, const std::vector<cv::KeyPoint> keypoints1,
                       const cv::Mat *image2, const std::vector<cv::KeyPoint> keypoints2,
                       const std::vector<cv::DMatch> matches) const;
    Image *drawKeypoints(const cv::Mat *image, const std::vector<cv::KeyPoint> keypoints) const;
public:
    FeatureMatcher();
    virtual ~FeatureMatcher();

    Image *getKeypoints(const cv::Mat *image) const;
    Image *getLargestScaleKeypoints(const cv::Mat *image) const;
    Image *match(const cv::Mat *queryImage, const cv::Mat *trainImage) const;
    Image *matchLargestScaleKeypoint(const cv::Mat *queryImage, const cv::Mat *trainImage) const;
    Image *bagOfWords(const cv::Mat *image, const char *outputCsvFilename) const;
    Image *matchBagOfWords(const cv::Mat *image, const cv::Mat *referenceImages,
                           const char *outputCsvFilename) const;
};

#endif
