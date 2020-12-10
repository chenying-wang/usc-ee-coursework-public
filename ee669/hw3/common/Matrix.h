#ifndef __EE669_MATRIX
#define __EE669_MATRIX

#include <iostream>
#include <vector>
#include <functional>

namespace ee669 {
    template<typename T>
    class Matrix {
    private:
        const uint64_t width;
        const uint64_t height;
        std::vector<T> data;

    public:
        class iterator {
        protected:
            Matrix<T> *matrix;

        public:
            iterator();
            virtual ~iterator();

            virtual void forEach(const std::function<void(T &)> &fn) const;
            iterator *set(Matrix<T> *matrix) {
                this->matrix = matrix;
                return this;
            }
        };

        class zigzagIterator : public iterator {
        public:
            zigzagIterator();
            virtual ~zigzagIterator();

            void forEach(const std::function<void(T &)> &fn) const override;
        };

        class zOrderIterator : public iterator {
        private:
            bool test() const;
            void iterate(const std::function<void(T &)> &fn, const uint64_t size,
                const uint64_t xOffset, const uint64_t yOffset) const;

        public:
            zOrderIterator();
            virtual ~zOrderIterator();

            void forEach(const std::function<void(T &)> &fn) const override;
        };

        class hilbertIterator : public iterator {
        private:
            bool test() const;
            void iterate(const std::function<void(T &)> &fn, const uint64_t size,
                const uint64_t xOffset, const uint64_t yOffset,
                const int64_t x1, const int64_t y1,
                const int64_t x2, const int64_t y2) const;

        public:
            hilbertIterator();
            virtual ~hilbertIterator();

            void forEach(const std::function<void(T &)> &fn) const override;
        };

        class blockingIterator : public iterator {
        private:
            const uint64_t blockWidth;
            const uint64_t blockHeight;

        public:
            blockingIterator(const uint64_t blockWidth, const uint64_t blockHeight);
            virtual ~blockingIterator();

            void forEach(const std::function<void(T &)> &fn) const override;
        };

        Matrix(const uint64_t width, const uint64_t height);
        virtual ~Matrix();

        template<typename Iterator>
        Matrix<T> *load(Iterator first, Iterator last);
        Matrix<T> *release();

        T &operator[](const std::pair<uint64_t, uint64_t> &pos);
        const T &operator[](const std::pair<uint64_t, uint64_t> &pos) const;

        uint64_t getWidth() const {
            return this->width;
        }

        uint64_t getHeight() const {
            return this->height;
        }

        std::vector<T> getData() {
            return this->data;
        }
    };

    template<typename T>
    Matrix<T>::Matrix(const uint64_t width, const uint64_t height) :
        width(width), height(height) {
        this->data.resize(width * height);
    }

    template<typename T>
    Matrix<T>::~Matrix() {
        this->release();
    }

    template<typename T>
    template<typename Iterator>
    Matrix<T> *
    Matrix<T>::load(Iterator first, Iterator last) {
        this->data.clear();
        this->data.insert(this->data.begin(), first, last);
        return this;
    }

    template<typename T>
    Matrix<T> *
    Matrix<T>::release() {
        this->data.clear();
        return this;
    }

    template<typename T>
    T &
    Matrix<T>::operator[](const std::pair<uint64_t, uint64_t> &pos) {
        return this->data[pos.first * this->width + pos.second];
    }

    template<typename T>
    const T &
    Matrix<T>::operator[](const std::pair<uint64_t, uint64_t> &pos) const {
        return this->data[pos.first * this->width + pos.second];
    }

    template<typename T>
    Matrix<T>::iterator::iterator() {}

    template<typename T>
    Matrix<T>::iterator::~iterator() {}

    template<typename T>
    void
    Matrix<T>::iterator::forEach(const std::function<void(T &)> &fn) const {
        for (auto &data : this->matrix->data) {
            fn(data);
        }
    }

    template<typename T>
    Matrix<T>::zigzagIterator::zigzagIterator() {}

    template<typename T>
    Matrix<T>::zigzagIterator::~zigzagIterator() {}

    template<typename T>
    void
    Matrix<T>::zigzagIterator::forEach(const std::function<void(T &)> &fn) const {
        bool direction = false;
        uint64_t i = 0UL, j = 0UL;
        while (i < this->matrix->height && j < this->matrix->width) {
            fn((*this->matrix)[std::make_pair(i, j)]);
            if (direction) {
                if (i + 1 == this->matrix->height) {
                    ++j;
                    direction = !direction;
                } else if (!j) {
                    ++i;
                    direction = !direction;
                } else {
                    ++i; --j;
                }
            } else {
                if (j + 1 == this->matrix->width) {
                    ++i;
                    direction = !direction;
                } else if (!i) {
                    ++j;
                    direction = !direction;
                } else {
                    --i; ++j;
                }
            }
        }
    }

    template<typename T>
    Matrix<T>::zOrderIterator::zOrderIterator() {}

    template<typename T>
    Matrix<T>::zOrderIterator::~zOrderIterator() {}

    template<typename T>
    void
    Matrix<T>::zOrderIterator::forEach(const std::function<void(T &)> &fn) const {
        if (!this->test()) {
            return;
        }
        this->iterate(fn, this->matrix->width, 0UL, 0UL);
    }

    template<typename T>
    void
    Matrix<T>::zOrderIterator::iterate(const std::function<void(T &)> &fn, const uint64_t size,
            const uint64_t xOffset, const uint64_t yOffset) const {
        if (size == 1) {
            fn((*this->matrix)[std::make_pair(xOffset, yOffset)]);
            return;
        }
        const uint64_t nextSize = size >> 1;
        this->iterate(fn, nextSize, xOffset, yOffset);
        this->iterate(fn, nextSize, xOffset, yOffset + nextSize);
        this->iterate(fn, nextSize, xOffset + nextSize, yOffset);
        this->iterate(fn, nextSize, xOffset + nextSize, yOffset + nextSize);
    }

    template<typename T>
    bool
    Matrix<T>::zOrderIterator::test() const {
        if (this->matrix->width != this->matrix->height) {
            return false;
        }
        int size = this->matrix->width ;
        while (size > 1) {
            if (size % 2) {
                return false;
            }
            size >>= 1;
        }
        return true;
    }

    template<typename T>
    Matrix<T>::hilbertIterator::hilbertIterator() {}

    template<typename T>
    Matrix<T>::hilbertIterator::~hilbertIterator() {}

    template<typename T>
    void
    Matrix<T>::hilbertIterator::forEach(const std::function<void(T &)> &fn) const {
        if (!this->test()) {
            return;
        }
        int64_t x1 = this->matrix->width >> 1, y1 = 0;
        int64_t x2 = 0, y2 = this->matrix->width >> 1;
        this->iterate(fn, this->matrix->width, 0UL, 0UL, x1, y1, x2, y2);
    }

    template<typename T>
    bool
    Matrix<T>::hilbertIterator::test() const {
        if (this->matrix->width != this->matrix->height) {
            return false;
        }
        int size = this->matrix->width ;
        while (size > 1) {
            if (size % 2) {
                return false;
            }
            size >>= 1;
        }
        return true;
    }

    template<typename T>
    void
    Matrix<T>::hilbertIterator::iterate(const std::function<void(T &)> &fn, const uint64_t size,
            const uint64_t xOffset, const uint64_t yOffset,
            const int64_t x1, const int64_t y1,
            const int64_t x2, const int64_t y2) const {
        if (size == 1) {
            fn((*this->matrix)[std::make_pair(xOffset, yOffset)]);
            return;
        }
        const uint64_t nextSize = size >> 1;
        this->iterate(fn, nextSize, xOffset, yOffset, x2 >> 1, y2 >> 1, x1 >> 1, y1 >> 1);
        this->iterate(fn, nextSize, xOffset + x1, yOffset + y1, x1 >> 1, y1 >> 1, x2 >> 1, y2 >> 1);
        this->iterate(fn, nextSize, xOffset + x1 + x2, yOffset + y1 + y2, x1 >> 1, y1 >> 1, x2 >> 1, y2 >> 1);
        this->iterate(fn, nextSize, xOffset + x1, yOffset + y1, -x2 >> 1, -y2 >> 1, -x1 >> 1, -y1 >> 1);
    }

    template<typename T>
    Matrix<T>::blockingIterator::blockingIterator(const uint64_t blockWidth, const uint64_t blockHeight) :
        blockWidth(blockWidth), blockHeight(blockHeight) {}

    template<typename T>
    Matrix<T>::blockingIterator::~blockingIterator() {}

    template<typename T>
    void
    Matrix<T>::blockingIterator::forEach(const std::function<void(T &)> &fn) const {
        if (!this->blockWidth || !this->blockHeight) {
            for (auto &data : this->matrix->data) {
                fn(data);
            }
            return;
        }

        for (auto i = 0UL; i < this->matrix->height; i += this->blockHeight) {
            for (auto j = 0UL; j < this->matrix->width; j += this->blockWidth) {
                for (auto k = 0UL; k < this->blockHeight && i + k < this->matrix->height; ++k) {
                    for (auto l = 0UL; l < this->blockWidth && j + l < this->matrix->width; ++l) {
                        fn((*this->matrix)[std::make_pair(i + k, j + l)]);
                    }
                }
            }
        }
    }
}

#endif
