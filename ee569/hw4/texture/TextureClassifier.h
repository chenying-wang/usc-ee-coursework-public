/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __TEXTURE_CLASSIFIER
#define __TEXTURE_CLASSIFIER

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

class TextureClassifier {
private:
    std::vector<float> m_pfNormMean;
    std::vector<float> m_pfNormStdDev;
    std::vector<std::vector<float>> m_ppfPcaVector;

protected:
    TextureClassifier();

    std::unordered_multimap<std::string, std::vector<float>>
        trainNorm(const std::unordered_multimap<std::string, std::vector<float>> dataset);
    std::unordered_multimap<std::string, std::vector<float>>
        trainPca(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                 const uint dimension);
    std::vector<std::vector<float>> norm(const std::vector<std::vector<float>> features) const;
    std::vector<std::vector<float>> pca(const std::vector<std::vector<float>> features) const;
    float dot(const std::vector<float> v1, const std::vector<float> v2) const;
    float distanceSquare(const std::vector<float> v1, const std::vector<float> v2) const;

public:
    virtual ~TextureClassifier();

    virtual TextureClassifier *train(const std::unordered_multimap<std::string, std::vector<float>> dataset,
                                     const uint pca = 0u);
    virtual std::vector<std::string> estimate(const std::vector<std::vector<float>> features) const;
};

#endif
