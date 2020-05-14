/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __IMAGE_STITCHING
#define __IMAGE_STITCHING

#include <iostream>
#include <vector>

#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"

#include "ImageCvAdapter.h"
#include "common/Image.h"
#include "common/ImageIO.h"

class ImageStitching {
private:
    ImageCvAdapter m_cImageCvAdapter;
    ImageIO m_cImageIO;

    double *solveTransformation(std::vector<double *> from,
                                std::vector<double *> to) const;
    double *transform(const double x, const double y, const double *transform) const;
    std::vector<cv::DMatch> findBestmatches(std::vector<std::vector<cv::DMatch>> knnMatches,
                                            std::vector<cv::KeyPoint> keypoints2) const;
    Image *drawMatches(const cv::Mat *image1, const std::vector<cv::KeyPoint> keypoints1,
                       const cv::Mat *image2, const std::vector<cv::KeyPoint> keypoints2,
                       const std::vector<std::vector<cv::DMatch>> knnMatches1to2,
                       const std::vector<cv::DMatch> bestMatches1to2,
                       const bool swap) const;

public:
    ImageStitching();
    virtual ~ImageStitching();

    Image *stitch(const Image *leftImage, const Image *middleImage, const Image *rightImage,
                  const char *matchesLMFilename, const char *matchesMRFilename,
                  const char *bestMatchesLMFilename, const char *bestMatchesMRFilename) const;
};

#endif
