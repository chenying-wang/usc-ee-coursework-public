/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#ifndef __EDGE_DETECTION
#define __EDGE_DETECTION

#include <iostream>

#include "common/Filter.h"
#include "common/Image.h"
#include "common/ImageConverter.h"
#include "common/ImageIO.h"

class EdgeDetection {
private:
    Filter *m_pcXSobelFilter;
    Filter *m_pcYSobelFilter;
    ImageConverter m_cImageConverter;
    ImageIO m_cImageIO;

public:
    EdgeDetection();
    virtual ~EdgeDetection();

    EdgeDetection *create();
    EdgeDetection *destroy();

    Image *detectEdgeBySobelFilter(const Image *image, const PIXEL_TYPE threshold,
                                   const char *xGradientMapOuput, const char *yGradientMapOuput,
                                   const char *gradientMapOuput) const;
    Image *detectEdgeByCannyDetecteor(const Image *image,
                                      const PIXEL_TYPE lowThreshold, const PIXEL_TYPE highThreshold) const;
};

#endif
