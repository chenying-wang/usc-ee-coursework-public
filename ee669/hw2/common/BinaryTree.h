#ifndef __EE669_BINARY_TREE
#define __EE669_BINARY_TREE

#include <iostream>

template<typename T>
class BinaryTree {
private:
    T value;
    BinaryTree<T> *left;
    BinaryTree<T> *right;

public:
    BinaryTree();
    BinaryTree(T &value);
    BinaryTree(T &value, BinaryTree<T> *left, BinaryTree<T> *right);
    virtual ~BinaryTree();

    const T &getValue() const {
        return this->value;
    }

    T &getValue() {
        return this->value;
    }

    void setValue(T &value) {
        this->value = value;
    }

    BinaryTree<T> *getLeft() {
        return this->left;
    }

    BinaryTree<T> *getRight() {
        return this->right;
    }

    bool isLeaf() const {
        return this->left == nullptr && this->right == nullptr;
    }

};

template<typename T>
BinaryTree<T>::BinaryTree() {}

template<typename T>
BinaryTree<T>::BinaryTree(T &value) :
    value(value), left(nullptr), right(nullptr) {}

template<typename T>
BinaryTree<T>::BinaryTree(T &value, BinaryTree<T> *left, BinaryTree<T> *right) :
    value(value), left(left), right(right) {}

template<typename T>
BinaryTree<T>::~BinaryTree() {
    delete this->left;
    delete this->right;
}

#endif
