#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <cmath>
#include <climits>

#include "DCTNode.h"
#include "common/Matrix.h"

using ee669::DCTNode;

std::array<std::array<uint64_t, 8UL>, 8UL>
DCTNode::Q_50 = {{
    {16, 11, 10, 16,  24,  40,  51,  61},
    {12, 12, 14, 19,  26,  58,  60,  55},
    {14, 13, 16, 24,  40,  57,  69,  56},
    {14, 17, 22, 29,  51,  87,  80,  62},
    {18, 22, 37, 56,  68, 109, 103,  77},
    {24, 35, 55, 64,  81, 104, 113,  92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103,  99}
}};

DCTNode::DCTNode(const std::string &key, const std::string &inputNode,
        const uint64_t width, const uint64_t height,
        const uint64_t blockSize, const uint8_t qualityFactor) :
    ee669::Node<std::vector<uint8_t>>(key), inputNode(inputNode),
    width(width), height(height), blockSize(std::max(blockSize, 1UL)),
    qualityFactor(std::min(qualityFactor, static_cast<uint8_t>(100))) {}

DCTNode::~DCTNode() {}

std::vector<uint8_t> *
DCTNode::eval(Session *session) const {
    auto input = this->template getDependency<std::vector<uint8_t>>(session, this->inputNode);
    if (input == nullptr || input->size() != this->width * this->height) {
        return input;
    }

    const double qualityParameter = !this->qualityFactor ? 0.0 :
        this->qualityFactor <= 50 ? 50.0 / this->qualityFactor : (100.0 - this->qualityFactor) / 50.0 ;
    const double sqrtBlockSizeInv = 1.0 / sqrt(this->blockSize);

    Matrix<int32_t> matrix(this->width, this->height);
    matrix.load(input->cbegin(), input->cend());
    for (auto i = 0UL; i < this->height; ++i) {
        for (auto j = 0UL; j < this->width; ++j) {
            matrix[std::make_tuple(i, j, 0)] -= 128;
        }
    }
    auto *result = new std::vector<uint8_t>();
    result->reserve(input->size());
    result->reserve(qualityParameter ? input->size() : input->size() * sizeof(double));
    for (auto x = 0UL; x < this->height; ++x) {
        for (auto y = 0UL; y < this->width; ++y) {
            auto i = x % this->blockSize, j = y % this->blockSize;
            double d = 0.0;
            for (auto k = 0UL; k < this->blockSize; ++k) {
                for (auto l = 0UL; l < this->blockSize; ++l) {
                    auto _x = x - i + k, _y = y - j + l;
                    d += std::cos((2 * _x + 1) * i * M_PI / this->blockSize / 2) *
                        std::cos((2 * _y + 1) * j * M_PI / this->blockSize / 2) *
                        matrix[std::make_tuple(_x, _y, 0)];
                }
            }
            d *= M_SQRT1_2 * sqrtBlockSizeInv;
            if (i == 0 && j == 0) {
                d *= 0.5;
            } else if (i == 0 || j == 0) {
                d *= M_SQRT1_2;
            }
            if (qualityParameter) {
                result->push_back(std::lround(d / DCTNode::Q_50[i][j] / qualityParameter));
            } else {
                result->insert(result->end(), reinterpret_cast<uint8_t *>(&d),
                    reinterpret_cast<uint8_t *>(&d) + sizeof(d));
            }
        }
    }

    matrix.release();
    return result;
}