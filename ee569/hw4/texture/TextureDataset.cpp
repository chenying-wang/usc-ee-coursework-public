/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>

#include "TextureDataset.h"

TextureDataset::TextureDataset() : m_mtxWrite(new std::mutex()) {}

TextureDataset::~TextureDataset() {
    this->clear();
}

std::unordered_multimap<std::string, std::vector<float>> TextureDataset::packageTrainingData() const {
    auto mTrainingData = std::unordered_multimap<std::string, std::vector<float>>();
    const auto size = this->size();
    if (!size) {
        return mTrainingData;
    }
    for (auto i = 0u; i < size; ++i) {
        mTrainingData.insert(std::make_pair(this->m_psLabel.at(i), this->m_ppfFeatures.at(i)));
    }
    return mTrainingData;
}

void TextureDataset::insert(std::string filename, std::vector<float> features, std::string label) {
    this->m_mtxWrite->lock();
    this->m_psFilename.push_back(filename);
    this->m_ppfFeatures.push_back(features);
    this->m_psLabel.push_back(label);
    this->m_mtxWrite->unlock();
}

std::tuple<std::string, std::vector<float>, std::string> TextureDataset::at(const uint i) const {
    return std::make_tuple(this->m_psFilename.at(i), this->m_ppfFeatures.at(i), this->m_psLabel.at(i));
}

uint TextureDataset::size() const {
    return this->m_psFilename.size();
}

void TextureDataset::clear() {
    this->m_psFilename.clear();
    this->m_ppfFeatures.clear();
    this->m_psLabel.clear();
}
