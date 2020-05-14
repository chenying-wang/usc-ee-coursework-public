/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

#include "ImageStitching.h"

#define HESSIAN_THRESHOLD (250.0)
#define RATIO_THRESHOLD (.3f)

ImageStitching::ImageStitching() {
}

ImageStitching::~ImageStitching() {
    // delete this->m_pcFeatureDetector;
    // delete this->m_pcDescriptorMatcher;
}

Image *ImageStitching::stitch(const Image *leftImage,
                              const Image *middleImage,
                              const Image *rightImage,
                              const char *matchesLMFilename,
                              const char *matchesMRFilename,
                              const char *bestMatchesLMFilename,
                              const char *bestMatchesMRFilename) const {
    if (leftImage == nullptr || leftImage->getImage() == nullptr ||
        middleImage == nullptr || middleImage->getImage() == nullptr ||
        rightImage == nullptr || rightImage->getImage() == nullptr) {
        return nullptr;
    }

    cv::Ptr<cv::Feature2D> pcFeatureDetector = cv::xfeatures2d::SURF::create(HESSIAN_THRESHOLD);
    cv::Ptr<cv::DescriptorMatcher> pcDescriptorMatcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);

    const auto pcLeftImgMat = this->m_cImageCvAdapter.toCvMat(leftImage);
    const auto pcMiddleImgMat = this->m_cImageCvAdapter.toCvMat(middleImage);
    const auto pcRightImgMat = this->m_cImageCvAdapter.toCvMat(rightImage);

    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
    std::vector<cv::KeyPoint> keypointsLeft, keypointsMiddle, keypointsRight;
    cv::Mat descriptorsLeft, descriptorsMiddle, descriptorsRight;
    pcFeatureDetector->detectAndCompute(*pcLeftImgMat, cv::noArray(), keypointsLeft, descriptorsLeft);
    pcFeatureDetector->detectAndCompute(*pcMiddleImgMat, cv::noArray(), keypointsMiddle, descriptorsMiddle);
    pcFeatureDetector->detectAndCompute(*pcRightImgMat, cv::noArray(), keypointsRight, descriptorsRight);

    //-- Step 2: Matching descriptor vectors with a FLANN based matcher
    // Since SURF is a floating-point descriptor NORM_L2 is used
    std::vector<std::vector<cv::DMatch>> knnMatchesML, knnMatchesMR;
    pcDescriptorMatcher->knnMatch(descriptorsMiddle, descriptorsLeft, knnMatchesML, 2);
    pcDescriptorMatcher->knnMatch(descriptorsMiddle, descriptorsRight, knnMatchesMR, 2);

    descriptorsLeft.release();
    descriptorsMiddle.release();
    descriptorsRight.release();

    std::cout << "Total matched key points between left and middle: " << knnMatchesML.size() << '\n';
    std::cout << "Total matched key points between middle and right: " << knnMatchesMR.size() << '\n';

    if (matchesLMFilename != nullptr) {
        const auto pcMatchesLMImage = this->drawMatches(pcLeftImgMat, keypointsLeft,
            pcMiddleImgMat, keypointsMiddle, knnMatchesML, std::vector<cv::DMatch>(), true);
        this->m_cImageIO.writeRawImage(pcMatchesLMImage, matchesLMFilename);
        delete pcMatchesLMImage;
    }
    if (matchesMRFilename != nullptr) {
        const auto pcMatchesMRImage = this->drawMatches(pcMiddleImgMat, keypointsMiddle,
            pcRightImgMat, keypointsRight, knnMatchesMR, std::vector<cv::DMatch>(), false);
        this->m_cImageIO.writeRawImage(pcMatchesMRImage, matchesMRFilename);
        delete pcMatchesMRImage;
    }

    auto pcBestMatchesML = this->findBestmatches(knnMatchesML, keypointsLeft);
    auto pcBestMatchesMR = this->findBestmatches(knnMatchesMR, keypointsRight);
    knnMatchesML.clear();
    knnMatchesMR.clear();

    if (matchesLMFilename != nullptr) {
        const auto pcBestMatchesLMImage = this->drawMatches(pcLeftImgMat, keypointsLeft,
            pcMiddleImgMat, keypointsMiddle, std::vector<std::vector<cv::DMatch>>(),
            pcBestMatchesML, true);
        this->m_cImageIO.writeRawImage(pcBestMatchesLMImage, bestMatchesLMFilename);
        delete pcBestMatchesLMImage;
    }
    if (matchesMRFilename != nullptr) {
        const auto pcBestMatchesMRImage = this->drawMatches(pcMiddleImgMat, keypointsMiddle,
            pcRightImgMat, keypointsRight, std::vector<std::vector<cv::DMatch>>(),
            pcBestMatchesMR, false);
        this->m_cImageIO.writeRawImage(pcBestMatchesMRImage, bestMatchesMRFilename);
        delete pcBestMatchesMRImage;
    }

    delete pcLeftImgMat;
    delete pcMiddleImgMat;
    delete pcRightImgMat;

    std::vector<double *> keypointsMLM, keypointsMLL;
    for (auto bestMatch : pcBestMatchesML) {
        auto keypointM = keypointsMiddle.at(bestMatch.queryIdx).pt;
        auto keypointL = keypointsLeft.at(bestMatch.trainIdx).pt;
        keypointsMLM.push_back(new double[2]{keypointM.y, keypointM.x});
        keypointsMLL.push_back(new double[2]{keypointL.y, keypointL.x});
    }
    pcBestMatchesML.clear();
    std::vector<double *> keypointsMRM, keypointsMRR;
    for (auto bestMatch : pcBestMatchesMR) {
        auto keypointM = keypointsMiddle.at(bestMatch.queryIdx).pt;
        auto keypointR = keypointsRight.at(bestMatch.trainIdx).pt;
        keypointsMRM.push_back(new double[2]{keypointM.y, keypointM.x});
        keypointsMRR.push_back(new double[2]{keypointR.y, keypointR.x});
    }
    pcBestMatchesMR.clear();

    const auto pdTransformationLM = this->solveTransformation(keypointsMLL, keypointsMLM);
    const auto pdTransformationML = this->solveTransformation(keypointsMLM, keypointsMLL);
    keypointsMLM.clear();
    keypointsMLL.clear();
    const auto pdTransformationRM = this->solveTransformation(keypointsMRR, keypointsMRM);
    const auto pdTransformationMR = this->solveTransformation(keypointsMRM, keypointsMRR);
    keypointsMRM.clear();
    keypointsMRR.clear();

    auto xMin = INT32_MAX, xMax = INT32_MIN;
    auto yMin = INT32_MAX, yMax = INT32_MIN;
    auto pVertices = new std::vector<double *>();
    pVertices->push_back(this->transform(0, 0, pdTransformationLM));
    pVertices->push_back(this->transform(leftImage->getHeight() - 1, 0, pdTransformationLM));
    pVertices->push_back(this->transform(0, leftImage->getWidth() - 1, pdTransformationLM));
    pVertices->push_back(this->transform(leftImage->getHeight() - 1, leftImage->getWidth() - 1, pdTransformationLM));
    pVertices->push_back(new double[2]{.0, .0});
    pVertices->push_back(new double[2]{middleImage->getHeight() - 1., .0});
    pVertices->push_back(new double[2]{.0, middleImage->getWidth() - 1.});
    pVertices->push_back(new double[2]{middleImage->getHeight() - 1., middleImage->getWidth() - 1.});
    pVertices->push_back(this->transform(0, 0, pdTransformationRM));
    pVertices->push_back(this->transform(rightImage->getHeight() - 1, 0, pdTransformationRM));
    pVertices->push_back(this->transform(0, rightImage->getWidth() - 1, pdTransformationRM));
    pVertices->push_back(this->transform(rightImage->getHeight() - 1, rightImage->getWidth() - 1, pdTransformationRM));
    for (auto vertex : *pVertices) {
        xMin = std::min((int) std::floor(vertex[0]), xMin);
        xMax = std::max((int) std::ceil(vertex[0]), xMax);
        yMin = std::min((int) std::floor(vertex[1]), yMin);
        yMax = std::max((int) std::ceil(vertex[1]), yMax);
    }
    delete pVertices;

    const auto uiWidth = yMax - yMin + 1;
    const auto uiHeight = xMax - xMin + 1;
    const auto uiChannel = middleImage->getChannel();
    const auto pcCanvasImage = (new Image(uiWidth, uiHeight, uiChannel * 3 + 1, leftImage->getDepth(),
        PaddingType::ALL_BLACK))->create();
    const auto pppuhLeftImage = leftImage->getImage();
    const auto pppuhMiddleImage = middleImage->getImage();
    const auto pppuhRightImage = rightImage->getImage();
    const auto pppuhCanvasImage = pcCanvasImage->getImage();

    pcCanvasImage->rasterScan([=](unsigned int i, unsigned int j) {
        const auto x = (int) i + xMin;
        const auto y = (int) j + yMin;

        const auto transformedPosL = this->transform(x, y, pdTransformationML);
        const auto xLeft = transformedPosL[0];
        const auto xLeftOffset = xLeft - std::floor(xLeft);
        const auto yLeft = transformedPosL[1];
        const auto yLeftOffset = yLeft - std::floor(yLeft);
        if (xLeft >= 0 && yLeft >= 0 &&
            xLeft < leftImage->getHeight() - 1 && yLeft < leftImage->getWidth() - 1) {
            for (auto k = 0u; k < uiChannel; ++k) {
                pppuhCanvasImage[i][j][k] =
                    (1 - xLeftOffset) * (1 - yLeftOffset) * pppuhLeftImage[(int) xLeft][(int) yLeft][k] +
                    (1 - xLeftOffset) * yLeftOffset * pppuhLeftImage[(int) xLeft][(int) yLeft + 1][k] +
                    xLeftOffset * (1 - yLeftOffset) * pppuhLeftImage[(int) xLeft + 1][(int) yLeft][k] +
                    xLeftOffset * yLeftOffset * pppuhLeftImage[(int) xLeft + 1][(int) yLeft + 1][k];
            }
            ++pppuhCanvasImage[i][j][3 * uiChannel];
        }

        if (x >= 0 && y >= 0 &&
            x < (int) middleImage->getHeight() && y < (int) middleImage->getWidth()) {
            for (auto k = 0u; k < uiChannel; ++k) {
                pppuhCanvasImage[i][j][uiChannel + k] = pppuhMiddleImage[x][y][k];
            }
            ++pppuhCanvasImage[i][j][3 * uiChannel];
        }

        const auto transformedPosR = this->transform(x, y, pdTransformationMR);
        const auto xRight = transformedPosR[0];
        const auto xRightOffset = xRight - std::floor(xRight);
        const auto yRight = transformedPosR[1];
        const auto yRightOffset = yRight - std::floor(yRight);
        if (xRight >= 0 && yRight >= 0 &&
            xRight < rightImage->getHeight() - 1 && yRight < rightImage->getWidth() - 1) {
            for (auto k = 0u; k < uiChannel; ++k) {
                pppuhCanvasImage[i][j][2 * uiChannel + k] =
                    (1 - xRightOffset) * (1 - yRightOffset) * pppuhRightImage[(int) xRight][(int) yRight][k] +
                    (1 - xRightOffset) * yRightOffset * pppuhRightImage[(int) xRight][(int) yRight + 1][k] +
                    xRightOffset * (1 - yRightOffset) * pppuhRightImage[(int) xRight + 1][(int) yRight][k] +
                    xRightOffset * yRightOffset * pppuhRightImage[(int) xRight + 1][(int) yRight + 1][k];
            }
            ++pppuhCanvasImage[i][j][3 * uiChannel];
        }
    });

    const auto pcProcessedImage = (new Image(uiWidth, uiHeight, uiChannel, leftImage->getDepth(), PaddingType::ALL_BLACK))->create();
    const auto pppuhProcessedImage = pcProcessedImage->getImage();
    pcProcessedImage->rasterScan([pppuhProcessedImage, pppuhCanvasImage, uiChannel](unsigned int i, unsigned int j) {
        if (!pppuhCanvasImage[i][j][3 * uiChannel]) {
            return;
        }
        for (auto k = 0u; k < uiChannel; ++k) {
            unsigned int sum = 0u;
            for (auto l = 0u; l < 3u; ++l) {
                sum += pppuhCanvasImage[i][j][l * uiChannel + k];
            }
            pppuhProcessedImage[i][j][k] = std::round(sum / pppuhCanvasImage[i][j][3 * uiChannel]);
        }
    });
    delete pcCanvasImage;

    return pcProcessedImage;
}

std::vector<cv::DMatch> ImageStitching::findBestmatches(std::vector<std::vector<cv::DMatch>> knnMatches,
                                                        std::vector<cv::KeyPoint> keypoints2) const {
    //-- Filter matches using the Lowe's ratio test
    const auto goodMatches = new std::vector<cv::DMatch>();
    for (auto knnMatch : knnMatches) {
        if (knnMatch[0].distance < RATIO_THRESHOLD * knnMatch[1].distance) {
            goodMatches->push_back(knnMatch[0]);
        }
    }

    auto dTL = std::numeric_limits<double>::max(), dTR = std::numeric_limits<double>::max();
    auto dBL = std::numeric_limits<double>::max(), dBR = std::numeric_limits<double>::max();
    const auto bestMatches = new cv::DMatch[4];
    for (auto goodMatch : *goodMatches) {
        auto keypointLeft = keypoints2.at(goodMatch.trainIdx).pt;
        auto x = keypointLeft.y;
        auto y = keypointLeft.x;
        if (x + y < dTL) {
            bestMatches[0] = goodMatch;
            dTL = x + y;
        } else if (x - y < dTR) {
            bestMatches[1] = goodMatch;
            dTR = x - y;
        } else if (-x + y < dBL) {
            bestMatches[2] = goodMatch;
            dBL = -x + y;
        } else if (-x - y < dBR) {
            bestMatches[3] = goodMatch;
            dBR = -x - y;
        }
    }
    goodMatches->clear();
    return std::vector<cv::DMatch>(bestMatches, bestMatches + 4);
}

double *ImageStitching::solveTransformation(std::vector<double *> from,
                                            std::vector<double *> to) const {
    if (from.size() != 4 || to.size() != 4) {
        return nullptr;
    }

    const auto pdMat = new double*[8];
    for (auto i = 0u; i < 4u; ++i) {
        auto j = i << 1;
        pdMat[j] = new double[9];
        pdMat[j][0] = -from.at(i)[0];
        pdMat[j][1] = -from.at(i)[1];
        pdMat[j][2] = -1;
        pdMat[j][3] = 0;
        pdMat[j][4] = 0;
        pdMat[j][5] = 0;
        pdMat[j][6] = from.at(i)[0] * to.at(i)[0];
        pdMat[j][7] = from.at(i)[1] * to.at(i)[0];
        pdMat[j][8] = to.at(i)[0];

        pdMat[j + 1] = new double[9];
        pdMat[j + 1][0] = 0;
        pdMat[j + 1][1] = 0;
        pdMat[j + 1][2] = 0;
        pdMat[j + 1][3] = -from.at(i)[0];
        pdMat[j + 1][4] = -from.at(i)[1];
        pdMat[j + 1][5] = -1;
        pdMat[j + 1][6] = from.at(i)[0] * to.at(i)[1];
        pdMat[j + 1][7] = from.at(i)[1] * to.at(i)[1];
        pdMat[j + 1][8] = to.at(i)[1];
    }

    const auto pcMat = new cv::Mat(8, 9, CV_64F);
    for (auto i = 0u; i < 8u; ++i) {
        for (auto j = 0u; j < 9u; ++j) {
            pcMat->at<double>(i, j) = pdMat[i][j];
        }
        delete[] pdMat[i];
    }
    delete[] pdMat;

    const auto pcSVD = new cv::SVD(*pcMat, cv::SVD::Flags::FULL_UV);
    delete pcMat;

    const auto pcSolution = pcSVD->vt;
    const auto pdSolution = new double[9];
    for (auto i = 0u; i < 9u; ++i) {
        pdSolution[i] = pcSolution.at<double>(8, i);
    }
    delete pcSVD;
    return pdSolution;
}

double *ImageStitching::transform(const double x, const double y, const double *transform) const {
    const auto x1 = transform[0] * x + transform[1] * y + transform[2];
    const auto y1 = transform[3] * x + transform[4] * y + transform[5];
    const auto w = transform[6] * x + transform[7] * y + transform[8];
    return new double[2]{x1 / w, y1 / w};
}

Image *ImageStitching::drawMatches(const cv::Mat *image1, const std::vector<cv::KeyPoint> keypoints1,
                                   const cv::Mat *image2, const std::vector<cv::KeyPoint> keypoints2,
                                   const std::vector<std::vector<cv::DMatch>> knnMatches1to2,
                                   const std::vector<cv::DMatch> bestMatches1to2,
                                   const bool swap) const {

    const auto pcMatches = new std::vector<cv::DMatch>();
    if (!knnMatches1to2.empty()) {
        for (auto pcMatch : knnMatches1to2) {
            auto cMatch = pcMatch.front();
            if (swap) {
                cv::DMatch cSwapMatch(cMatch.trainIdx, cMatch.queryIdx, cMatch.imgIdx, cMatch.distance);
                pcMatches->push_back(cSwapMatch);
                continue;
            }
            cv::DMatch cCpyMatch(cMatch.queryIdx, cMatch.trainIdx, cMatch.imgIdx, cMatch.distance);
            pcMatches->push_back(cCpyMatch);
        }
    } else if (!bestMatches1to2.empty()) {
        for (auto cMatch : bestMatches1to2) {
            if (swap) {
                cv::DMatch cSwapMatch(cMatch.trainIdx, cMatch.queryIdx, cMatch.imgIdx, cMatch.distance);
                pcMatches->push_back(cSwapMatch);
                continue;
            }
            cv::DMatch cCpyMatch(cMatch.queryIdx, cMatch.trainIdx, cMatch.imgIdx, cMatch.distance);
            pcMatches->push_back(cCpyMatch);
        }
    } else {
        return nullptr;
    }

    const auto pcMathesMat  = new cv::Mat();
    cv::drawMatches(*image1, keypoints1, *image2, keypoints2, *pcMatches, *pcMathesMat,
        cv::Scalar::all(-1), cv::Scalar::all(-1), std::vector<char>(),
        cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    delete pcMatches;

    const auto pcMatchesImage = this->m_cImageCvAdapter.fromCvMat(pcMathesMat);
    delete pcMathesMat;

    return pcMatchesImage;
}
