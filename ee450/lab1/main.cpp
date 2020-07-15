#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <sstream>
#include <string>
#include <limits>
#ifdef __TIME_STAT
#include <chrono>
#endif

#define SUCCESS 0
#define ERROR -1

#define MAX(T) std::numeric_limits<T>::max()

#define OUTPUT_FILE "./result.txt"

#ifndef __EE450_GRAPH
#define __EE450_GRAPH

template<typename T>
class Graph {
private:
    const uint64_t m_size;
    std::vector<std::string> m_vertices;
    std::unordered_map<std::string, uint64_t> m_vertexMapping;
    std::vector<std::vector<T>> m_matrix;
    uint64_t m_idx;

    const uint64_t getVertexIdx(const std::string v) const;
    const uint64_t getOrInsertVertexIdx(const std::string v);

public:
    Graph(const uint64_t size);
    virtual ~Graph();

    Graph<T> *addEdge(const std::string s, const std::string t, T distance);
    Graph<T> *addUndirectedEdge(const std::string v1, const std::string v2, T distance);
    std::unordered_map<uint64_t, uint64_t> calculateDistances(const std::string s, const std::string t) const;

    uint64_t getSize() const {
        return this->m_size;
    }
};

#endif

template<typename T>
Graph<T>::Graph(const uint64_t size) :
    m_size(size),
    m_vertices(std::vector<std::string>(size)),
    m_vertexMapping(std::unordered_map<std::string, uint64_t>(size)),
    m_matrix(std::vector<std::vector<T>>(size)),
    m_idx(0UL) {
    for (auto i = 0UL; i < this->m_size; ++i) {
        this->m_matrix[i] = std::vector<uint8_t>(this->m_size, MAX(T));
        this->m_matrix[i][i] = 0UL;
    }
}

template<typename T>
Graph<T>::~Graph() {
    this->m_vertices.clear();
    this->m_vertexMapping.clear();
    for (auto v : this->m_matrix) {
        v.clear();
    }
    this->m_matrix.clear();
}

template<typename T>
Graph<T> *Graph<T>::addEdge(const std::string s, const std::string t, T distance) {
    const auto sIdx = this->getOrInsertVertexIdx(s);
    const auto tIdx = this->getOrInsertVertexIdx(t);
    if (sIdx >= this->m_size || tIdx >= this->m_size) {
        std::cerr << "Failed to add edge from " << s << " to " << t << '\n';
        return this;
    }
    this->m_matrix[sIdx][tIdx] = distance;
    return this;
}

template<typename T>
Graph<T> *Graph<T>::addUndirectedEdge(const std::string v1, const std::string v2, T distance) {
    const auto v1Idx = this->getOrInsertVertexIdx(v1);
    const auto v2Idx = this->getOrInsertVertexIdx(v2);
    if (v1Idx >= this->m_size || v2Idx >= this->m_size) {
        std::cerr << "Failed to add edge between " << v1 << " and " << v2 << '\n';
        return this;
    }
    this->m_matrix[v1Idx][v2Idx] = distance;
    this->m_matrix[v2Idx][v1Idx] = distance;
    return this;
}

template<typename T>
std::unordered_map<uint64_t, uint64_t> Graph<T>::calculateDistances(const std::string s, const std::string t) const {
    const auto sIdx = this->getVertexIdx(s);
    const auto tIdx = this->getVertexIdx(t);
    if (sIdx >= this->m_size || tIdx >= this->m_size) {
        return std::unordered_map<uint64_t, uint64_t>();
    } else if (s == t) {
        return std::unordered_map<uint64_t, uint64_t>({{0, 1}});
    }

    const auto size = this->m_vertices.size();
    auto currentPath = new std::unordered_map<uint64_t, std::deque<std::unordered_set<uint64_t>>>();
    auto nextPath = new std::unordered_map<uint64_t, std::deque<std::unordered_set<uint64_t>>>();
    auto results = std::unordered_map<uint64_t, uint64_t>(size);
    nextPath->insert(std::make_pair(sIdx, std::deque<std::unordered_set<uint64_t>>()));
    nextPath->at(sIdx).push_back(std::unordered_set<uint64_t>({sIdx}));
    for (auto distance = 1UL; distance < size; ++distance) {
        std::swap(currentPath, nextPath);
        for (auto it = currentPath->cbegin(); it != currentPath->cend(); ++it) {
            const auto i = it->first;
            for (auto j = 0UL; j < size; ++j) {
                if (this->m_matrix[i][j] == MAX(T) || i == j) {
                    continue;
                } else if (j == tIdx) {
                    if (results.count(distance)) {
                        results.at(distance) += it->second.size();
                    } else {
                        results.insert(std::make_pair(distance, it->second.size()));
                    }
                    continue;
                }

                for (const auto path : it->second) {
                    if (path.count(j)) {
                        continue;
                    }
                    auto copyPath = path;
                    copyPath.insert(j);
                    if (!nextPath->count(j)) {
                        nextPath->insert(std::make_pair(j, std::deque<std::unordered_set<uint64_t>>()));
                    }
                    nextPath->at(j).push_back(copyPath);
                }
            }
        }

        if (nextPath->empty()) {
            break;
        }
        for (auto it = currentPath->begin(); it != currentPath->end(); ++it) {
            for (auto path : it->second) {
                path.clear();
            }
            it->second.clear();
        }
        currentPath->clear();
    }

    for (auto it = currentPath->begin(); it != currentPath->end(); ++it) {
        for (auto path : it->second) {
            path.clear();
        }
        it->second.clear();
    }
    currentPath->clear();
    delete currentPath;
    for (auto it = nextPath->begin(); it != nextPath->end(); ++it) {
        for (auto path : it->second) {
            path.clear();
        }
        it->second.clear();
    }
    nextPath->clear();
    delete nextPath;

    if (results.empty()) {
        return std::unordered_map<uint64_t, uint64_t>();
    }
    return results;
}

template<typename T>
const uint64_t Graph<T>::getVertexIdx(const std::string v) const {
    if (this->m_vertexMapping.empty() || !this->m_vertexMapping.count(v)) {
        return MAX(uint64_t);
    }
    return this->m_vertexMapping.at(v);
}

template<typename T>
const uint64_t Graph<T>::getOrInsertVertexIdx(const std::string v) {
    if (this->m_vertexMapping.count(v)) {
        return this->m_vertexMapping[v];
    } else if (this->m_idx >= this->m_size) {
        return MAX(uint64_t);
    }
    this->m_vertices[this->m_idx] = v;
    this->m_vertexMapping.insert(std::make_pair(v, this->m_idx));
    return this->m_idx++;
}

Graph<uint8_t> *readGraph(std::istream &in, uint64_t size) {
    if (!in.good()) {
        return nullptr;
    }
    auto graph = new Graph<uint8_t>(size);
    std::string line;
    while (std::getline(in, line)) {
        if (line.size() < 3) {
            continue;
        }
        std::istringstream iss(line);
        std::string v1, v2;
        iss >> v1 >> v2;
        if (!iss.eof()) {
            continue;
        }
        graph->addUndirectedEdge(v1, v2, 1u);
    }
    return graph;
}

int main(int argc, char *argv[]) {
#ifdef __TIME_STAT
    auto start = std::chrono::system_clock::now();
#endif
    if (argc < 2) {
        std::cerr << "Unspecified the input file\n";
        return ERROR;
    }
    auto fin = std::ifstream(argv[1], std::ios::in);
    if (!fin.good()) {
        std::cerr << "Cannot open the input file: " << argv[1] << '\n';
        fin.close();
        return ERROR;
    }

    uint64_t n;
    std::string line, s, t;
    fin >> n >> s >> t;
    auto graph = readGraph(fin, n << 1);
    fin.close();

    auto results = graph->calculateDistances(s, t);
    auto fout = std::ofstream(OUTPUT_FILE, std::ios::out);
    for (auto result : results) {
        if (result.first < 2) {
            for (auto i = 0UL; i < result.second; ++i) {
                fout << "0\n";
            }
            continue;
        }
        const auto p = std::to_string(result.first - 1) + '\n';
        for (auto i = 0UL; i < result.second; ++i) {
            fout << p;
        }
    }
    fout.close();
#ifdef __TIME_STAT
    auto elapsed = std::chrono::system_clock::now() - start;
    std::cerr << "Elapsed time: " << elapsed.count() / 1000000.0 << "ms\n";
#endif
    return SUCCESS;
}
