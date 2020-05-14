/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#ifndef __MORPHOLOGICAL_FILTER
#define __MORPHOLOGICAL_FILTER

#include <iostream>

#include "MorphologicalMaskReader.h"
#include "common/Filter.h"
#include "common/Image.h"
#include "common/ImageConverter.h"
#include "common/ImageIO.h"
#include "common/ImageStat.h"

class MorphologicalFilter {
private:
    ImageConverter m_cImageConverter;
    ImageIO m_cImageIO;
    ImageStat m_cImageStat;
    const MorphologicalMaskReader *m_pcMorphologicalMaskReader1;
    const MorphologicalMaskReader *m_pcMorphologicalMaskReader2;
    const Filter *m_pcSurroundingFilter;

    Image *morphologicalProcess(const Image *image, const unsigned int iteration,
                                const std::vector<MorphologicalMask> stage1Masks,
                                const std::vector<MorphologicalMask> stage2Masks,
                                const char *trackFilename) const;
    void dfs(const Image *image, const unsigned int x, const unsigned int y,
             const int i, const int j, bool **visited, const unsigned int maxRadius,
             const std::function<void(int k, int l)>& fn,
             const bool eightBond = false) const;


public:
    MorphologicalFilter(const char *stage1MasksFilename, const char *stage2MasksFilename);
    virtual ~MorphologicalFilter();

    MorphologicalFilter *create();
    MorphologicalFilter *destroy();

    Image *shrink(const Image *image, const unsigned int iteration,
                  const char *trackFilename) const;
    Image *thin(const Image *image, const unsigned int iteration,
                const char *trackFilename) const;
    Image *skeletonize(const Image *image, const unsigned int iteration,
                       const char *trackFilename) const;
    Image *count(const Image *image, const unsigned int iteration, const char *sizeCsvFilename) const;
    Image *countByDfs(const Image *image, const char *sizeCsvFilename) const;
    Image *pcbHole(const Image *image, const unsigned int iteration) const;
    Image *pcbPathway(const Image *image, const unsigned int iteration, const char *trackFilename) const;
    Image *detectDefect(const Image *image, const unsigned int iteration, const char *trackFilename) const;
};

#endif
