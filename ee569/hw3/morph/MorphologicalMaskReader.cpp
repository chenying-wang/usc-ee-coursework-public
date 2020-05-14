/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>
#include <fstream>
#include <cstring>

#include "MorphologicalMaskReader.h"
#include "MorphologicalMask.h"

MorphologicalMaskReader::MorphologicalMaskReader(const char *masksCsvFilename) :
    m_phMasksCsvFilename(masksCsvFilename) {
    this->create();
}

MorphologicalMaskReader::~MorphologicalMaskReader() {
    this->destroy();
}

MorphologicalMaskReader *MorphologicalMaskReader::create() {
    this->m_ppcMasks = new std::vector<MorphologicalMask>[std::strlen(MASKS_LIST)];
    return this;
}

MorphologicalMaskReader *MorphologicalMaskReader::destroy() {
    for (auto i = 0u; i < std::strlen(MASKS_LIST); ++i) {
        // while (!this->m_ppcMasks[i].empty()) {
        //     delete &(this->m_ppcMasks[i].back());
        //     this->m_ppcMasks[i].pop_back();
        // }
    }
    delete[] this->m_ppcMasks;
    this->m_ppcMasks = nullptr;
    return this;
}

void MorphologicalMaskReader::readMasksFromCsv(const char *typeList) const {
    std::ifstream fInput;
    fInput.open(this->m_phMasksCsvFilename, std::ios::in);

    auto uiLine = 0u;
    while (!fInput.eof()) {
        auto phLine = new char[64];
        fInput.getline(phLine, 64);
        if (fInput.fail()) {
            std::cerr << "Error: Failed to read " << this->m_phMasksCsvFilename << '\n';
            fInput.close();
            break;
        }
        auto len = std::strlen(phLine);
        if (!len || phLine[0] == '#') {
            delete[] phLine;
            continue;
        }

        auto pphFields = new char*[3];
        pphFields[0] = new char[10];
        std::memset(pphFields[0], 0x00, 10 * sizeof(char));
        auto j = 0u;
        for (auto i = 0u, k = 0u; i < len; ++i) {
            if (phLine[i] == ',') {
                pphFields[++j] = new char[10];
                std::memset(pphFields[j], 0x00, 10 * sizeof(char));
                k = 0u;
                continue;
            }
            pphFields[j][k++] = phLine[i];
        }
        delete[] phLine;

        if (j != 2) {
            for (auto i = 0u; i <= j; ++i) {
                delete[] pphFields[i];
            }
            delete[] pphFields;
            continue;
        }

        const auto sTypeListLen = std::strlen(typeList);
        auto phHitTypeList = new char[sTypeListLen];
        std::memset(phHitTypeList, 0x00, sTypeListLen * sizeof(char));
        for (auto i = 0u, j = 0u; i < sTypeListLen; ++i) {
            if (std::strchr(pphFields[0], typeList[i])) {
                phHitTypeList[j++] = typeList[i];
            }
        }
        delete[] pphFields[0];

        const unsigned int uiSpin = atoi(pphFields[1]);
        delete[] pphFields[1];

        if (!std::strlen(phHitTypeList) || uiSpin < 1 || uiSpin > 4) {
            delete[] pphFields[2];
            delete[] pphFields;
            continue;
        }

        for (auto i = 0u; i < uiSpin; ++i) {
            auto phMasks = new uint8_t[8];
            for (auto j = 0u; j < 8u; ++j) {
                const auto ch = pphFields[2][j];
                if (ch == '0' ) {
                    phMasks[(j + i * 2) % 8] = 0u;
                } else if (ch == '1' || ch == 'M') {
                    phMasks[(j + i * 2) % 8] = 1u;
                } else if (ch == 'X') {
                    phMasks[(j + i * 2) % 8] = 2u;
                } else {
                    phMasks[(j + i * 2) % 8] = 3u;
                }
            }
            const auto pcMorphologicalMask = new MorphologicalMask(new const uint8_t*[3]{
                new const uint8_t[3]{phMasks[3], phMasks[2], phMasks[1]},
                new const uint8_t[3]{phMasks[4], 1u, phMasks[0]},
                new const uint8_t[3]{phMasks[5], phMasks[6], phMasks[7]},
            });
            delete[] phMasks;

            for (auto i = 0u; i < std::strlen(MASKS_LIST); ++i) {
                if (!std::strchr(phHitTypeList, MASKS_LIST[i])) {
                    continue;
                }
                this->m_ppcMasks[i].push_back(*pcMorphologicalMask);
            }
        }
        delete[] pphFields[2];
        delete[] pphFields;
        ++uiLine;
    }
    std::cout << "Total " << uiLine << " lines read from " << this->m_phMasksCsvFilename << '\n';

    fInput.close();
}

void MorphologicalMaskReader::print() const {
    std::ofstream fOutput;
    fOutput.open("mask.txt", std::ios::out);
    for (auto i = 0u; i < std::strlen(MASKS_LIST); ++i) {
        fOutput << MASKS_LIST[i] << '\n';
        auto masks = this->m_ppcMasks[i];
        for (auto mask : masks) {
            for (auto j = 0u; j < 3u; ++j) {
                for (auto k = 0u; k < 3u; ++k) {
                    fOutput << (uint) mask.getMask()[j][k] << ' ';
                }
                fOutput << '\n';
            }
            fOutput << '\n';
        }
    }
}

std::vector<MorphologicalMask> *MorphologicalMaskReader::getMasks(const char *type) const {
    auto len = std::strlen(type);
    std::string phUnread;
    auto ppcMasks = new std::vector<MorphologicalMask>[len];
    for (auto i = 0u; i < len; ++i) {
        auto idx = std::strchr(MASKS_LIST, type[i]) - MASKS_LIST;
        if (this->m_ppcMasks[idx].empty()) {
            phUnread.push_back(type[i]);
        }
    }
    if (!phUnread.empty()) {
        this->readMasksFromCsv(phUnread.c_str());
    }
    for (auto i = 0u; i < len; ++i) {
        auto idx = std::strchr(MASKS_LIST, type[i]) - MASKS_LIST;
        ppcMasks[i] = this->m_ppcMasks[idx];
    }
    return ppcMasks;
}
