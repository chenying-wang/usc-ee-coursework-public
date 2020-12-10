#ifndef __EE669_SCALAR_QUANTIZER
#define __EE669_SCALAR_QUANTIZER

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <cmath>

#include "common/StreamProcess.h"

namespace ee669 {
    template<typename T>
    class ScalarQuantizer : public StreamProcess<T> {
    private:
        static_assert(std::is_integral<T>::value, "Template type T must be integral");

        const uint8_t codebookSize;
        const T maxVal;
        const T minVal;
        const double epsilon;
        const uint64_t maxSteps;
        std::vector<T> codebook;

        std::vector<T> process0(std::vector<T> &input) override;
        void init();

    public:
        ScalarQuantizer(const uint8_t codebookSize, const T maxVal, const T minVal,
            const double epsilon, const uint64_t maxSteps);
        virtual ~ScalarQuantizer();

        ScalarQuantizer<T> *build(const std::unordered_map<T, uint64_t> &stat);
    };

    template<typename T>
    ScalarQuantizer<T>::ScalarQuantizer(const uint8_t codebookSize, const T maxVal, const T minVal,
            const double epsilon, const uint64_t maxSteps) :
        StreamProcess<T>(), codebookSize(codebookSize),
        maxVal(maxVal), minVal(minVal), epsilon(epsilon), maxSteps(maxSteps) {}

    template<typename T>
    ScalarQuantizer<T>::~ScalarQuantizer() {
        this->codebook.clear();
    }

    template<typename T>
    ScalarQuantizer<T> *ScalarQuantizer<T>::build(const std::unordered_map<T, uint64_t> &stat) {
        if (this->codebookSize >= 1 << CHAR_BIT * sizeof(T)) {
            return this;
        }
        this->init();

        uint64_t streamSize = 0UL;
        for (const auto &[symbol, count] : stat) {
            streamSize += count;
        }

        std::map<T, uint64_t> sortedStat(stat.cbegin(), stat.cend());
        std::vector<std::vector<T>> cluster;
        double prevMse = -1.0, mse = -1.0;
        for (int i = 0; i < this->codebookSize; ++i) {
            cluster.push_back(std::vector<T>());
        }
        std::vector<T> interval(this->codebookSize, minVal);
        std::vector<T> tempCodebook;
        tempCodebook.reserve(this->codebookSize);
        for (auto step = 0U; step < this->maxSteps; ++step) {
            for (auto i = 1U; i < this->codebook.size(); ++i) {
                interval[i] = ((static_cast<uint64_t>(this->codebook[i - 1]) +
                    static_cast<uint64_t>(this->codebook[i]) + 1UL) >> 1);
            }
            uint8_t c = 0U;
            cluster[0].clear();
            for (auto it = sortedStat.cbegin(); it != sortedStat.cend(); ++it) {
                while (c + 1U < this->codebook.size() && it->first >= interval[c + 1]) {
                    cluster[++c].clear();
                }
                cluster[c].push_back(it->first);
            }

            for (auto i = 0U; i <= c; ++i) {
                if (cluster[i].empty()) {
                    continue;
                }
                uint64_t weightedSum = 0UL, count = 0UL;
                for (const auto val : cluster[i]) {
                    weightedSum += val * stat.at(val);
                    count += stat.at(val);
                }
                tempCodebook.push_back(0.5 + static_cast<double>(weightedSum) / count);
            }

            mse = 0.0;
            for (auto i = 0U; i < tempCodebook.size(); ++i) {
                for (const auto val : cluster[i]) {
                    double error = val - tempCodebook[i];
                    mse += error * error * stat.at(val);
                }
            }
            mse /= streamSize;
            if (prevMse > 0.0 && mse >= prevMse) {
                break;
            }
            this->codebook = tempCodebook;
            tempCodebook.clear();

            double psnr = 10 * std::log10(static_cast<double>(this->maxVal - this->minVal) *
                (this->maxVal - this->minVal) / mse);
            std::clog << "Step: " << step
                << ", CLUSTER: " << this->codebook.size()
                << ", MSE: " << mse
                << ", PSNR: " << psnr << '\n';
            if (prevMse > 0.0 && prevMse - mse < this->epsilon * mse) {
                break;
            }
            prevMse = mse;
        }
        return this;
    }

    template<typename T>
    std::vector<T> ScalarQuantizer<T>::process0(std::vector<T> &input) {
        if (this->codebookSize >= 1 << CHAR_BIT * sizeof(T)) {
            return input;
        }
        std::vector<T> quantized;
        if (input.empty()) {
            return quantized;
        }

        std::vector<T> interval(this->codebookSize, minVal);
        for (auto i = 1U; i < this->codebookSize; ++i) {
            interval[i] = (static_cast<uint64_t>(this->codebook[i - 1]) + static_cast<uint64_t>(this->codebook[i])) >> 1;
        }

        quantized.reserve(input.size());
        for (const auto symbol : input) {
            uint8_t c = 0U;
            while (c + 1 < this->codebookSize && symbol >= interval[c + 1]) {
                ++c;
            }
            quantized.push_back(static_cast<T>(this->codebook[c]));
        }
        return quantized;
    }


    template<typename T>
    void ScalarQuantizer<T>::init() {
        this->codebook.assign(this->codebookSize, this->minVal);
        for (uint64_t i = 0; i < this->codebookSize; ++i) {
            this->codebook[i] += (2 * i + 1) * (this->maxVal - this->minVal) / this->codebookSize >> 1;
        }
    }
}

#endif
