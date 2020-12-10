#ifndef __EE669_UTILS
#define __EE669_UTILS

#include <iostream>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <cmath>
#include <climits>

namespace ee669 {
    class Utils {
    private:
        Utils();

    public:
        virtual ~Utils();

        template<typename Iterator, typename T = typename std::iterator_traits<Iterator>::value_type>
        static std::unordered_map<T, uint64_t> stat(Iterator first, Iterator last);

        template<typename T>
        static double calcEntropy(const std::unordered_map<T, uint64_t> &stat);

        template<typename Iterator, typename T = typename std::iterator_traits<Iterator>::value_type>
        static double calcPSNR(Iterator first, Iterator last,
            Iterator oriFirst, Iterator oriLast, T max = std::numeric_limits<T>::max());
    };

    template<typename Iterator, typename T>
    std::unordered_map<T, uint64_t> Utils::stat(Iterator first, Iterator last) {
        auto count = std::unordered_map<T, uint64_t>();
        while (first != last) {
            ++count[*first];
            ++first;
        }
        return count;
    }

    template<typename T>
    double Utils::calcEntropy(const std::unordered_map<T, uint64_t> &stat) {
        static_assert(std::is_integral<T>::value, "Template type T must be integral");
        if (stat.empty()) {
            return 0.0;
        }
        uint64_t numSymbols = 0UL;
        for (const auto &e : stat) {
            numSymbols += e.second;
        }
        double entropy = .0;
        for (const auto &e : stat) {
            double prob = (double) e.second / numSymbols;
            entropy -= prob * std::log2(prob);
        }
        return entropy;
    }

    template<typename Iterator, typename T>
    double Utils::calcPSNR(Iterator first, Iterator last, Iterator oriFirst, Iterator oriLast, T max) {
        static_assert(std::is_integral<T>::value, "Template type T must be integral");
        const auto size = std::distance(first, last);
        if (std::distance(first, last) != std::distance(oriFirst, oriLast)) {
            return 0.0;
        }
        uint64_t mse = 0UL;
        while (first != last) {
            auto error = static_cast<int64_t>(*first) - static_cast<int64_t>(*oriFirst);
            mse += error * error;
            ++first;
            ++oriFirst;
        }
        double dMse = static_cast<double>(mse) / size;
        return 10 * std::log10(static_cast<double>(max) * max / dMse);
    }
}

#endif
