/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __CV_TEXTURE_CLASSIFIER
#define __CV_TEXTURE_CLASSIFIER

#include <iostream>
#include <vector>
#include <unordered_map>

#include "opencv2/core.hpp"
#include "opencv2/ml.hpp"

#include "TextureClassifier.h"

#define TEXTURE_OPENCV_MODEL_RF 1
#define TEXTURE_OPENCV_MODEL_SVM 2

class CvTextureClassifier : public TextureClassifier {
private:
    const uint m_uiType;
    cv::Ptr<cv::ml::StatModel> m_pcCvModel;
    std::unordered_map<std::string, uint> m_mLabelMapping;
    std::unordered_map<uint, std::string> m_mLabelReverseMapping;

public:
    CvTextureClassifier(const uint type);
    virtual ~CvTextureClassifier();

    TextureClassifier *train(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                             const uint pca = 0u);
    std::vector<std::string> estimate(const std::vector<std::vector<float>> features) const;
};

#endif
