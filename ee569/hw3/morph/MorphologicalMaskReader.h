/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __MORPHOLOGICAL_MASK_READER
#define __MORPHOLOGICAL_MASK_READER

#include <iostream>
#include <vector>

#include "MorphologicalMask.h"

#define MASKS_LIST ("STK")

class MorphologicalMaskReader {
private:
    const char *m_phMasksCsvFilename;
    std::vector<MorphologicalMask> *m_ppcMasks;

public:
    MorphologicalMaskReader(const char *masksCsvFilename);
    virtual ~MorphologicalMaskReader();

    MorphologicalMaskReader *create();
    MorphologicalMaskReader *destroy();

    void readMasksFromCsv(const char *typeList) const;
    void print() const;
    std::vector<MorphologicalMask> *getMasks(const char *type) const;
};

#endif
