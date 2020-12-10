#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cstddef>

#include "Code.h"
#include "Encoder.h"
#include "common/BinaryTree.h"

#ifndef __EE669_HUFFMAN_ENCODER
#define __EE669_HUFFMAN_ENCODER

template<typename T>
class HuffmanEncoder : public Encoder<T> {
private:
    typedef BinaryTree<Code<T>> _TreeT;

    template<typename ...Tp>
    void buildTree(std::priority_queue<_TreeT *, Tp...> &leaves) const;
    void buildCodebook(_TreeT *root);

public:
    HuffmanEncoder();
    virtual ~HuffmanEncoder();

    Encoder<T> *build(const std::unordered_map<T, uint64_t> &stat) override;
};

template<typename T>
HuffmanEncoder<T>::HuffmanEncoder() {}

template<typename T>
HuffmanEncoder<T>::~HuffmanEncoder() {}

template<typename T>
Encoder<T> *
HuffmanEncoder<T>::build(const std::unordered_map<T, uint64_t> &stat) {
    this->codebook.clear();
    if (stat.empty()) {
        return this;
    } else if (stat.size() == 1) {
        auto c = Code<T>(stat.cbegin()->second, stat.cbegin()->first);
        c.codeLen = 1;
        this->codebook.insert(std::make_pair(stat.cbegin()->first, c));
        return this;
    }

    const static auto comparator = [](_TreeT *a, _TreeT *b) {
        return std::greater<uint64_t>()(a->getValue().frequency, b->getValue().frequency);
    };
    auto leaves = std::priority_queue<_TreeT *, std::vector<_TreeT *>, decltype(comparator)>(comparator);
    for (const auto &e : stat) {
        auto code = Code<T>(e.second, e.first);
        leaves.push(new _TreeT(code));
    }
    this->buildTree(leaves);
    const auto root = leaves.top();
    this->buildCodebook(root);
    delete root;
    return this->calcEntropy(stat);
}

template<typename T>
template<typename ...Tp>
void
HuffmanEncoder<T>::buildTree(std::priority_queue<_TreeT *, Tp...> &leaves) const {
    while (leaves.size() > 1) {
        auto left = leaves.top();
        leaves.pop();
        auto right = leaves.top();
        leaves.pop();
        auto code = Code<T>(left->getValue().frequency + right->getValue().frequency);
        leaves.push(new _TreeT(code, left, right));
    }
}

template<typename T>
void
HuffmanEncoder<T>::buildCodebook(_TreeT *root) {
    const auto &huffmanCode = root->getValue();
    if (root->isLeaf()) {
        this->codebook.insert(std::make_pair(huffmanCode.symbol, huffmanCode));
        return;
    }
    auto &lHuffmanCode = root->getLeft()->getValue();
    auto &rHuffmanCode = root->getRight()->getValue();
    lHuffmanCode.code = huffmanCode.code << 1;
    lHuffmanCode.codeLen = huffmanCode.codeLen + 1;
    rHuffmanCode.code = lHuffmanCode.code;
    rHuffmanCode.code.set(0);
    rHuffmanCode.codeLen = lHuffmanCode.codeLen;
    this->buildCodebook(root->getLeft());
    this->buildCodebook(root->getRight());
}

#endif
