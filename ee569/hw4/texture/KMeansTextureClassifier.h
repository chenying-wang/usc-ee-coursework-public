/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __K_MEANS_TEXTURE_CLASSIFIER
#define __K_MEANS_TEXTURE_CLASSIFIER

#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>

#include "TextureClassifier.h"

class KMeansTextureClassifier : public TextureClassifier {
private:
    uint m_uiK = 0u;
    bool m_bNorm = false;
    std::vector<std::vector<float>> m_ppfClusterCentroids;
    std::vector<std::string> m_psLables;

    std::vector<std::vector<float>> init(const std::vector<std::vector<float>> features) const;

public:
    KMeansTextureClassifier(const uint K, const bool norm);
    virtual ~KMeansTextureClassifier();

    TextureClassifier *train(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                             const uint pca = 0u);
    std::vector<std::string> estimate(const std::vector<std::vector<float>> features) const;
};



#endif
