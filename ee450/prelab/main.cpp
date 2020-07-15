#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <string>
#include <iterator>
#include <sstream>
#include <regex>
#include <numeric>
#include <limits>

#define SUCCESS 0
#define ERROR -1

#define MAX(T) std::numeric_limits<T>::max()
#define INF MAX(int64_t)

std::tuple<std::vector<int64_t>, std::vector<size_t>, uint> bellmanFord(std::vector<std::vector<std::vector<int64_t>>> distanceMat) {
    size_t size = distanceMat.size();
    if (!size) {
        return std::make_tuple(std::vector<int64_t>(), std::vector<size_t>(), 0u);
    }
    auto distance = std::vector<std::vector<int64_t>>();
    for (auto distanceVector : distanceMat) {
        if (distanceVector.size() != size) {
            return std::make_tuple(std::vector<int64_t>(), std::vector<size_t>(), 0u);
        }
        distance.push_back(std::vector<int64_t>());
        for (auto d : distanceVector) {
            auto sum = std::accumulate(d.cbegin(), d.cend(), 0L);
            distance.back().push_back(sum);
        }
    }

    auto d = std::vector<std::vector<int64_t>>();
    d.push_back(std::vector<int64_t>(size, INF));
    d[0][0] = 0L;
    auto p = std::vector<size_t>(size, 0L);
    auto it = size;
    auto x = -1L;
    for (auto i = 1u; i <= size; ++i) {
        d.push_back(std::vector<int64_t>(size, INF));
        auto et = true;
        for (auto j = 0u; j < size; ++j) {
            d[i][j] = d[i - 1][j];
        }
        for (auto j = 0u; j < size; ++j) {
            if (d[i - 1][j] == INF) {
                continue;
            }
            for (auto k = 0u; k < size; ++k) {
                if (j == k || distance[j][k] == INF) {
                    continue;
                }
                auto distanceTemp = distance[j][k] + d[i - 1][j];
                if (distanceTemp < d[i][k]) {
                    d[i][k] = distanceTemp;
                    p[k] = j;
                    et = false;
                    x = k;
                }
            }
        }
        if (et) {
            it = i;
            break;
        }
    }
    if (it < size || x < 0) {
        return std::make_tuple(d.back(), p, it);
    }

    return std::make_tuple(std::vector<int64_t>(1, x), p, it);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Unspecified input file\n";
        return ERROR;
    }
    std::ifstream fin;
    fin.open(argv[1], std::ios::in);
    if (!fin.good()) {
        std::cerr << "Cannot open file: " << argv[1] << '\n';
        fin.close();
        return ERROR;
    }

    auto distanceMat = std::vector<std::vector<std::vector<int64_t>>>();
    std::string line;
    while (std::getline(fin, line)) {
        if (line.size() < 2) {
            continue;
        }
        distanceMat.push_back(std::vector<std::vector<int64_t>>());
        std::istringstream iss(line);
        std::istream_iterator<std::string> it(iss);
        std::regex numberRegex("\\-?\\d+", std::regex_constants::optimize | std::regex_constants::ECMAScript);
        std::regex arrayRegex("\\[\\-?\\d+(?:,\\-?\\d+)+\\]", std::regex_constants::optimize | std::regex_constants::ECMAScript);
        std::regex searchRegex("(\\-?\\d+)", std::regex_constants::optimize | std::regex_constants::ECMAScript);
        while (it != std::istream_iterator<std::string>()) {
            if (std::regex_match(*it, numberRegex)) {
                distanceMat.back().push_back(std::vector<int64_t>(1, std::stol(*it)));
            } else if (std::regex_match(*it, arrayRegex)) {
                auto distanceVector = std::vector<int64_t>();
                auto s = *it;
                std::smatch sm;
                while (std::regex_search(s, sm, searchRegex)) {
                    distanceVector.push_back(std::stol(sm[1]));
                    s = sm.suffix().str();
                }
                distanceMat.back().push_back(distanceVector);
            } else {
                distanceMat.back().push_back(std::vector<int64_t>(1, INF));
            }
            ++it;
        }
    }
    fin.close();

    auto [d, p, it] = bellmanFord(distanceMat);
    if (it >= p.size()) {
        std::cout << "Negative Loop Detected\n";
        auto n = d[0];
        std::vector<size_t> queue;
        do {
            queue.push_back(n);
            n = p[n];
        } while (n != d[0]);
        std::cout << d[0];
        for (auto i = 0u; i < queue.size(); ++i) {
            std::cout << "->" << queue[queue.size() - i - 1];
        }
        std::cout << '\n';
        queue.clear();
        return SUCCESS;
    }

    std::cout << d[0];
    for (auto i = 1u; i < d.size(); ++i) {
        std::cout << ',' << d[i];
    }
    std::cout << '\n';

    for (auto i = 0u; i < p.size(); ++i) {
        auto n = i;
        std::vector<size_t> queue;
        while (n != 0) {
            queue.push_back(n);
            n = p[n];
        }
        std::cout << '0';
        for (auto i = 0u; i < queue.size(); ++i) {
            std::cout << "->" << queue[queue.size() - i - 1];
        }
        std::cout << '\n';
        queue.clear();
    }

    std::cout << "Iteration:" << it << '\n';

    return SUCCESS;
}
