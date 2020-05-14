/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 28, 2020
 **/
#include <iostream>
#include <algorithm>
#include <string>
#include <atomic>
#include <vector>
#include <cstring>
#include <cmath>

#include "MorphologicalFilter.h"

MorphologicalFilter::MorphologicalFilter(
    const char *stage1MasksFilename,
    const char *stage2MasksFilename) {
    this->m_pcMorphologicalMaskReader1 = new MorphologicalMaskReader(stage1MasksFilename);
    this->m_pcMorphologicalMaskReader2 = new MorphologicalMaskReader(stage2MasksFilename);
    this->create();
}

MorphologicalFilter::~MorphologicalFilter() {
    this->destroy();
}

MorphologicalFilter *MorphologicalFilter::create() {
    this->m_pcSurroundingFilter = new Filter(new const float *[9]{
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 0, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1},
        new const float[9]{1, 1, 1, 1, 1, 1, 1, 1, 1}
    }, 9, 9);
    return this;
}

MorphologicalFilter *MorphologicalFilter::destroy() {
    delete this->m_pcMorphologicalMaskReader1;
    delete this->m_pcMorphologicalMaskReader2;
    delete this->m_pcSurroundingFilter;
    return this;
}

Image *MorphologicalFilter::shrink(const Image *image, const unsigned int iteration, const char *trackFilename) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    const auto pcStage1Masks = this->m_pcMorphologicalMaskReader1->getMasks("S")[0];
    const auto pcStage2Masks = this->m_pcMorphologicalMaskReader2->getMasks("S")[0];
    return this->morphologicalProcess(image, iteration, pcStage1Masks, pcStage2Masks, trackFilename);
}

Image *MorphologicalFilter::thin(const Image *image, const unsigned int iteration, const char *trackFilename) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    const auto pcStage1Masks = this->m_pcMorphologicalMaskReader1->getMasks("T")[0];
    const auto pcStage2Masks = this->m_pcMorphologicalMaskReader2->getMasks("T")[0];
    return this->morphologicalProcess(image, iteration, pcStage1Masks, pcStage2Masks, trackFilename);
}

Image *MorphologicalFilter::skeletonize(const Image *image, const unsigned int iteration, const char *trackFilename) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    const auto pcStage1Masks = this->m_pcMorphologicalMaskReader1->getMasks("K")[0];
    const auto pcStage2Masks = this->m_pcMorphologicalMaskReader2->getMasks("K")[0];
    return this->morphologicalProcess(image, iteration, pcStage1Masks, pcStage2Masks, trackFilename);
}

Image *MorphologicalFilter::count(const Image *image, const unsigned int iteration,
                                  const char *sizeCsvFilename) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    const auto pcShrinkImage = this->shrink(image, iteration, nullptr);
    const auto puiCount = new std::atomic_uint(0u);
    const auto pppuhShrinkImage = pcShrinkImage->getImage();
    pcShrinkImage->concurrentScan([puiCount, pppuhShrinkImage](unsigned int i, unsigned int j) {
        if (pppuhShrinkImage[i][j][CHANNEL_GRAY]) {
            ++*puiCount;
        }
    });
    std::cout << "Number of Stars: "<< *puiCount << '\n';

    if (sizeCsvFilename == nullptr) {
        return pcShrinkImage;
    }

    const auto maxRadius = 16u;
    const auto maxSize = maxRadius * maxRadius << 2;
    const auto puiSizeDistribution = new unsigned int[maxSize];
    std::memset(puiSizeDistribution, 0x00, maxSize * sizeof(unsigned int));

    pcShrinkImage->rasterScan([=](unsigned int i, unsigned int j) {
        if (!pppuhShrinkImage[i][j][CHANNEL_GRAY]) {
            return;
        }
        auto size = new unsigned int(0);
        this->dfs(image, i, j, 0, 0, nullptr, maxRadius, [size](int, int) {
            ++*size;
        });
        ++puiSizeDistribution[*size];
    });
    if (sizeCsvFilename == nullptr) {
        return pcShrinkImage;
    }
    this->m_cImageStat.writeCsv(puiSizeDistribution, maxSize, sizeCsvFilename);
    return pcShrinkImage;
}

Image *MorphologicalFilter::countByDfs(const Image *image, const char *sizeCsvFilename) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    const auto puiCount = new unsigned int(0u);
    const auto maxRadius = 16u;
    const auto maxSize = maxRadius * maxRadius << 2;
    const auto puiSizeDistribution = new unsigned int[maxSize];
    std::memset(puiSizeDistribution, 0x00, maxSize * sizeof(unsigned int));

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    auto visited = new bool*[uiHeight];
    for (auto i = 0u; i < uiHeight; ++i) {
        visited[i] = new bool[uiWidth];
        std::memset(visited[i], 0x00, uiWidth * sizeof(bool));
    }
    image->rasterScan([=](unsigned int i, unsigned int j) {
        auto size = new unsigned int(0u);
        this->dfs(image, 0, 0, i, j, visited, std::max(uiWidth, uiHeight),
            [size](int, int) {
                ++*size;
            });
        if (!*size) {
            return;
        }
        ++puiSizeDistribution[*size];
        ++*puiCount;
    });
    if (sizeCsvFilename != nullptr) {
        this->m_cImageStat.writeCsv(puiSizeDistribution, maxSize, sizeCsvFilename);
    }
    std::cout << "Number of Stars: "<< *puiCount << '\n';

    return this->m_cImageConverter.duplicate(image);
}

Image *MorphologicalFilter::pcbHole(const Image *image, const unsigned int iteration) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    const auto pcShrinkImage = this->shrink(image, iteration, nullptr);
    const auto pppuhShrinkImage = pcShrinkImage->getImage();
    const auto puiCount = new std::atomic_uint(0u);
    pcShrinkImage->concurrentScan([=](unsigned i, unsigned j) {
        if (!pppuhShrinkImage[i][j][CHANNEL_GRAY]) {
            return;
        }
        if (!this->m_pcSurroundingFilter->filterImage(pcShrinkImage, i, j, CHANNEL_GRAY)) {
            ++*puiCount;
        }
    });
    std::cout << "Number of Holes: " << *puiCount << '\n';

    return pcShrinkImage;
}

Image *MorphologicalFilter::pcbPathway(const Image *image, const unsigned int iteration,
                                       const char *trackFilename) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();

    const auto pcOriImage = this->m_cImageConverter.duplicate(image);
    const auto pcHoleImage = this->shrink(image, iteration, nullptr);
    const auto pppuhOriImage = pcOriImage->getImage();
    const auto pppuhHoleImage = pcHoleImage->getImage();

    pcHoleImage->concurrentScan([=](unsigned int i, unsigned int j) {
        if (i < 2 || j < 2 || i > uiHeight - 3 || j > uiWidth - 3) {
            pppuhOriImage[i][j][CHANNEL_GRAY] = MAX_PIXEL_VALUE;
        }
        if (!pppuhHoleImage[i][j][CHANNEL_GRAY]) {
            return;
        }
        if (!this->m_pcSurroundingFilter->filterImage(pcHoleImage, i, j, CHANNEL_GRAY)) {
            this->dfs(image, i, j, 0, 0, nullptr, 16u, [pppuhOriImage](int k, int l) {
                pppuhOriImage[k][l][CHANNEL_GRAY] = 0u;
            });
        }
    });
    delete pcHoleImage;

    const auto pcInvertedImage = this->m_cImageConverter.invert(pcOriImage);
    if (trackFilename != nullptr) {
        this->m_cImageIO.clean(trackFilename);
        this->m_cImageIO.appendToRawImage(pcInvertedImage, trackFilename);
    }

    const auto pcThinImage = this->thin(pcInvertedImage, iteration, nullptr);
    if (trackFilename != nullptr) {
        this->m_cImageIO.appendToRawImage(pcThinImage, trackFilename);
    }
    const auto uiThresholdForIsolatedHoles = 10u;
    const auto puiIsolatedHolesCount = new unsigned int(0u);
    auto visited = new bool*[uiHeight];
    for (auto i = 0u; i < uiHeight; ++i) {
        visited[i] = new bool[uiWidth];
        std::memset(visited[i], 0x00, uiWidth * sizeof(bool));
    }
    pcThinImage->rasterScan([=](unsigned i, unsigned j) {
        auto size = new unsigned int(0u);
        this->dfs(pcThinImage, 0, 0, i, j, visited, std::max(uiWidth, uiHeight),
            [size, pcThinImage](int, int) {
                ++*size;
            }, true);
        if (*size  && *size < uiThresholdForIsolatedHoles) {
            ++*puiIsolatedHolesCount;
        }
    });
    delete pcThinImage;

    const auto pcShrinkImage = this->shrink(pcInvertedImage, iteration, nullptr);
    delete pcInvertedImage;
    if (trackFilename != nullptr) {
        this->m_cImageIO.appendToRawImage(pcShrinkImage, trackFilename);
    }
    const auto pppuhShrinkImage = pcShrinkImage->getImage();
    const auto puiPathwaysAndIsolatedHolesCount = new std::atomic_uint(0u);
    pcShrinkImage->concurrentScan([pppuhShrinkImage, puiPathwaysAndIsolatedHolesCount](unsigned i, unsigned j) {
        if (pppuhShrinkImage[i][j][CHANNEL_GRAY]) {
            ++*puiPathwaysAndIsolatedHolesCount;
        }
    });
    std::cout << "Number of Pathways: " << *puiPathwaysAndIsolatedHolesCount - *puiIsolatedHolesCount<< '\n';

    delete pcOriImage;
    return pcShrinkImage;
}

Image *MorphologicalFilter::detectDefect(const Image *image, const unsigned int iteration,
                                         const char *trackFilename) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr) {
        return nullptr;
    }

    // Invert and shrink
    const auto pcInvertedImage = this->m_cImageConverter.invert(image);
    const auto pcShrinkImage = this->shrink(pcInvertedImage, iteration, nullptr);
    const auto pppuhShrinkImage = pcShrinkImage->getImage();

    // Delete the holes by dfs
    const auto pcProcessedImage = this->m_cImageConverter.duplicate(image);
    const auto pppuhProcessedImage = pcProcessedImage->getImage();
    pcShrinkImage->concurrentScan([=](unsigned int i, unsigned int j) {
        if (!pppuhShrinkImage[i][j][CHANNEL_GRAY]) {
            return;
        }
        if (!this->m_pcSurroundingFilter->filterImage(pcShrinkImage, i, j, CHANNEL_GRAY)) {
            this->dfs(pcInvertedImage, i, j, 0, 0, nullptr, 32u, [pppuhProcessedImage](int k, int l) {
                pppuhProcessedImage[k][l][CHANNEL_GRAY] = MAX_PIXEL_VALUE;
            });
        }
    });
    if (trackFilename != nullptr) {
        this->m_cImageIO.clean(trackFilename);
        this->m_cImageIO.appendToRawImage(pcShrinkImage, trackFilename);
        this->m_cImageIO.appendToRawImage(pcProcessedImage, trackFilename);
    }
    delete pcInvertedImage;
    delete pcShrinkImage;

    // Get center pixel of gear by shrinking
    const auto pcShrinkProcessedImage = this->shrink(pcProcessedImage, iteration, nullptr);
    const auto pppuhShrinkProcessedImage = pcShrinkProcessedImage->getImage();
    const auto puiCenter = new unsigned int[2]{0u, 0u};
    pcShrinkProcessedImage->rasterScan([=](unsigned int i, unsigned int j) {
        if (pppuhShrinkProcessedImage[i][j][CHANNEL_GRAY]) {
            puiCenter[0] = i;
            puiCenter[1] = j;
        }
    });
    if (trackFilename != nullptr) {
        this->m_cImageIO.appendToRawImage(pcShrinkProcessedImage, trackFilename);
    }
    delete pcShrinkProcessedImage;

    // Find the boundary of gear
    const auto uiWidth = image->getWidth();
    const auto uiHeight = image->getHeight();
    const auto pcProcessedDuplicatedImage = this->m_cImageConverter.duplicate(pcProcessedImage);
    const auto pppuhProcessedDuplicatedImage = pcProcessedDuplicatedImage->getImage();
    this->dfs(pcProcessedDuplicatedImage, puiCenter[0], puiCenter[1], 0, 0, nullptr,
        1 + std::max(uiWidth / 2, uiHeight / 2), [pppuhProcessedImage, pppuhProcessedDuplicatedImage](int k, int l) {
            if (pppuhProcessedDuplicatedImage[k - 1][l][CHANNEL_GRAY] &&
                pppuhProcessedDuplicatedImage[k][l - 1][CHANNEL_GRAY] &&
                pppuhProcessedDuplicatedImage[k + 1][l][CHANNEL_GRAY] &&
                pppuhProcessedDuplicatedImage[k][l + 1][CHANNEL_GRAY]) {
                pppuhProcessedImage[k][l][CHANNEL_GRAY] = 0;
            }
        }
    );
    if (trackFilename != nullptr) {
        this->m_cImageIO.appendToRawImage(pcProcessedImage, trackFilename);
    }

    // Traverse the boundary to find distance
    const auto piMaxDistanceSquare = new int(0), piMinDistanceSquare = new int(INT32_MAX);
    pcProcessedImage->rasterScan([=](unsigned int i, unsigned int j) {
        if (!pppuhProcessedImage[i][j][CHANNEL_GRAY]) {
            return;
        }
        int x = (int) i - puiCenter[0], y = (int) j - puiCenter[1];
        auto distanceSquare = x * x + y * y;
        *piMaxDistanceSquare = std::max(distanceSquare, *piMaxDistanceSquare);
        *piMinDistanceSquare = std::min(distanceSquare, *piMinDistanceSquare);
    });
    const auto dMaxDistance = std::sqrt(*piMaxDistanceSquare);
    const auto dMinDistance = std::sqrt(*piMinDistanceSquare);
    const auto dStdDistanceSquare = 0.98 * *piMaxDistanceSquare + 0.02 * *piMinDistanceSquare;
    const auto dThreshold = 0.8 * dMaxDistance + 0.2 * dMinDistance;
    const auto dThresholdSquare = dThreshold * dThreshold;

    // Get angles where greater than distance threshold
    const auto pToothAngles = new std::vector<double>();
    pcProcessedImage->rasterScan([=](unsigned int i, unsigned int j) {
        if (!pppuhProcessedImage[i][j][CHANNEL_GRAY]) {
            return;
        }
        int x = i - puiCenter[0], y = j - puiCenter[1];
        auto distanceSquare = x * x + y * y;
        if (distanceSquare > dThresholdSquare) {
            auto atan = std::atan((0.0 + y) / x);
            if (x < 0) {
                atan += y > 0 ? M_PI : -M_PI;
            }
            pToothAngles->push_back(atan);
        }
    });
    delete pcProcessedImage;

    // Merge angles
    std::sort(pToothAngles->begin(), pToothAngles->end());
    const auto pTeeth = new std::vector<double *>();
    auto min = pToothAngles->front(), max = pToothAngles->front();
    for (auto i = 1u; i < pToothAngles->size(); ++i) {
        auto prevAngle = pToothAngles->at(i - 1);
        auto angle = pToothAngles->at(i);
        if (angle - prevAngle < 0.05) {
            max = angle;
            continue;
        }
        pTeeth->push_back(new double[3]{min, max});
        min = angle;
        max = angle;
    }
    if (std::abs(pToothAngles->back() - 2 * M_PI - pToothAngles->front()) < 0.05) {
        pTeeth->front()[0] = min;
    }
    pToothAngles->clear();
    delete pToothAngles;

    // Calculate angle of defects
    auto dAvgAngle = .0;
    auto size = pTeeth->size();
    for (auto i = 0u; i < size; ++i) {
        auto pdPos = pTeeth->at(i);
        if (pdPos[0] < pdPos[1]) {
            pdPos[2] = (pdPos[0] + pdPos[1]) / 2;
            dAvgAngle += pdPos[1] - pdPos[0];
            continue;
        }
        pdPos[2] = (pdPos[0] + pdPos[1] + 2 * M_PI) / 2;
        dAvgAngle += 2 * M_PI + pdPos[1] - pdPos[0];
        if (pdPos[2] > M_PI) {
            pdPos[2] -= M_PI;
        }
    }
    dAvgAngle /= size;
    const auto pStep = new std::vector<double>();
    for (auto i = 0u; i < size; ++i) {
        auto step = pTeeth->at((i + 1) % size)[2] - pTeeth->at(i)[2];
        step = step < 0 ? step + 2 * M_PI : step;
        pStep->push_back(step);
    }
    std::sort(pStep->begin(), pStep->end());
    auto dStdStep = .0;
    for (auto i = 0u; i < size / 2; ++i) {
        dStdStep += pStep->at(i);
    }
    dStdStep /= (size / 2);
    const auto dStepThreshold = pStep->front() * 1.5;
    const auto pDefects = new std::vector<double>();
    for (auto i = 0u; i < size; ++i) {
        auto angle = pTeeth->at(i)[2];
        auto step = pTeeth->at((i + 1) % size)[2] - angle;
        step = step < 0 ? step + 2 * M_PI : step;
        while (step > dStepThreshold) {
            angle += dStdStep;
            step -= dStdStep;
            pDefects->push_back(angle);
        }
    }
    pTeeth->clear();
    delete pTeeth;

    // Recover defects
    const auto pcRecoveredImage = this->m_cImageConverter.duplicate(image);
    const auto pppuhRecoveredImage = pcRecoveredImage->getImage();
    for (auto defect : *pDefects) {
        auto min = defect - dAvgAngle / 2;
        auto max = defect + dAvgAngle / 2;
        if (min < -M_PI) {
            min += 2 * M_PI;
        }
        if (max >= M_PI) {
            max -= 2 * M_PI;
        }
        pcRecoveredImage->rasterScan([=](unsigned int i, unsigned int j) {
            if (pppuhProcessedDuplicatedImage[i][j][CHANNEL_GRAY]) {
                return;
            }
            int x = i - puiCenter[0], y = j - puiCenter[1];
            auto distanceSquare = x * x + y * y;
            if (distanceSquare > dStdDistanceSquare) {
                return;
            }
            auto atan = std::atan((0.0 + y) / x);
            if (x < 0) {
                atan += y > 0 ? M_PI : -M_PI;
            }
            if (atan >= min && atan <= max) {
                pppuhRecoveredImage[i][j][CHANNEL_GRAY] = 0x7f;
            } else if (min > max && (atan <= max || atan >= min)) {
                pppuhRecoveredImage[i][j][CHANNEL_GRAY] = 0x7f;
            }
        });
    }
    delete pcProcessedDuplicatedImage;

    return pcRecoveredImage;
}

Image *MorphologicalFilter::morphologicalProcess(const Image *image, const unsigned int iteration,
                                                 const std::vector<MorphologicalMask> stage1Masks,
                                                 const std::vector<MorphologicalMask> stage2Masks,
                                                 const char *trackFilename) const {
    auto pcFromImage = this->m_cImageConverter.duplicate(image);

    if (trackFilename != nullptr) {
        this->m_cImageIO.clean(trackFilename);
        this->m_cImageIO.appendToRawImage(pcFromImage, trackFilename);
    }

    auto pcMarkImage = image->dumpStructure()->create();
    auto pcToImage = image->dumpStructure()->create();
    auto pppuhFromImage = pcFromImage->getImage();
    auto pppuhMarkImage = pcMarkImage->getImage();
    auto pppuhToImage = pcToImage->getImage();

    auto pbChange = new std::atomic_bool(false);
    auto i = 0u;
    for (i = 0u; i < iteration; ++i) {
        pcFromImage->concurrentScan([=](unsigned int i, unsigned int j) {
            if (!pppuhFromImage[i][j][CHANNEL_GRAY]) {
                pppuhMarkImage[i][j][CHANNEL_GRAY] = 0u;
                return;
            }
            for (const auto mask : stage1Masks) {
                if (mask.evaluate(pcFromImage, i, j, CHANNEL_GRAY)) {
                    pppuhMarkImage[i][j][CHANNEL_GRAY] = MAX_PIXEL_VALUE;
                    return;
                }
            }
            pppuhMarkImage[i][j][CHANNEL_GRAY] = 0u;
        });

        *pbChange = false;
        pcMarkImage->concurrentScan([=](unsigned int i, unsigned int j) {
            if (!pppuhMarkImage[i][j][CHANNEL_GRAY]) {
                pppuhToImage[i][j][CHANNEL_GRAY] = pppuhFromImage[i][j][CHANNEL_GRAY];
                return;
            }
            for (const auto mask : stage2Masks) {
                if (mask.evaluate(pcMarkImage, i, j, CHANNEL_GRAY)) {
                    pppuhToImage[i][j][CHANNEL_GRAY] = pppuhFromImage[i][j][CHANNEL_GRAY];
                    return;
                }
            }
            *pbChange = true;
            pppuhToImage[i][j][CHANNEL_GRAY] = 0u;
        });

        if (!*pbChange) {
            break;
        }

        if (trackFilename != nullptr) {
            this->m_cImageIO.appendToRawImage(pcFromImage, trackFilename);
        }

        const auto pcTemp = pcFromImage;
        pcFromImage = pcToImage;
        pcToImage = pcTemp;
        pppuhFromImage = pcFromImage->getImage();
        pppuhToImage = pcToImage->getImage();
    }
    std::cout << "Morphological process finish in " << i << " iterations\n";

    delete pcFromImage;
    delete pcMarkImage;
    return pcToImage;
}

void MorphologicalFilter::dfs(const Image *image, const unsigned int x, const unsigned int y,
                              const int i, const int j, bool **visited,
                              const unsigned int maxRadius,
                              const std::function<void(int k, int l)>& fn,
                              const bool eightBond) const {
    if (image == nullptr ||
        image->getChannel() != NUM_OF_CHANNELS_GRAY ||
        image->getImage() == nullptr ||
        i >= (int) maxRadius || j >= (int) maxRadius ||
        i < (int) -maxRadius || j < (int) -maxRadius) {
        return;
    }

    const auto maxRange = maxRadius << 1;
    auto _visited = visited;
    if (_visited == nullptr) {
        _visited = new bool*[maxRange];
        for (auto _i = 0u; _i < maxRange; ++_i) {
            _visited[_i] = new bool[maxRange];
            std::memset(_visited[_i], 0x00, maxRange * sizeof(bool));
        }
    }

    unsigned int vi = i < 0 ? i + maxRange : i;
    unsigned int vj = j < 0 ? j + maxRange : j;
    auto value = image->getPixel(x + i, y + j)[CHANNEL_GRAY];
    if (_visited[vi][vj] || !value) {
        return;
    }

    _visited[vi][vj] = true;
    fn(x + i, y + j);

    if (eightBond) {
        this->dfs(image, x, y, i - 1, j - 1, _visited, maxRadius, fn, eightBond);
        this->dfs(image, x, y, i - 1, j + 1, _visited, maxRadius, fn, eightBond);
        this->dfs(image, x, y, i + 1, j - 1, _visited, maxRadius, fn, eightBond);
        this->dfs(image, x, y, i + 1, j + 1, _visited, maxRadius, fn, eightBond);
    }
    this->dfs(image, x, y, i - 1, j, _visited, maxRadius, fn, eightBond);
    this->dfs(image, x, y, i, j + 1, _visited, maxRadius, fn, eightBond);
    this->dfs(image, x, y, i + 1, j, _visited, maxRadius, fn, eightBond);
    this->dfs(image, x, y, i, j - 1, _visited, maxRadius, fn, eightBond);
    return;
}
