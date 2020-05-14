/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __TEXTURE_DATASET
#define __TEXTURE_DATASET

#include <iostream>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <string>
#include <mutex>

class TextureDataset {
private:
    std::vector<std::string> m_psFilename;
    std::vector<std::vector<float>> m_ppfFeatures;
    std::vector<std::string> m_psLabel;
    std::mutex *m_mtxWrite;

public:
    TextureDataset();
    virtual ~TextureDataset();

    std::unordered_multimap<std::string, std::vector<float>> packageTrainingData() const;
    void insert(std::string filename, std::vector<float> features, std::string label);
    std::tuple<std::string, std::vector<float>, std::string> at(const uint i) const;
    uint size() const;
    void clear();

    std::vector<std::string> getFilename() const {
        return this->m_psFilename;
    }

    const std::vector<std::vector<float>> getFeatures() const {
        return this->m_ppfFeatures;
    }

    std::vector<std::string> getLabel() const {
        return this->m_psLabel;
    }
};

#endif
