#ifndef __EE669_MATRIX
#define __EE669_MATRIX

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

namespace ee669 {
    template<typename T>
    class Matrix {
    private:
        constexpr static T ZERO = T(0);
        const uint64_t width;
        const uint64_t height;
        const uint8_t channels;
        std::vector<T> data;

    public:
        enum Boundary {
            ZERO_SHIFT = 0,
            ODD_SYMMETERIC = 1,
            REPLICATION = 2
        };

        class iterator {
        protected:
            const uint8_t channel;
            Matrix<T> *matrix;

        public:
            iterator(const uint8_t channel);
            virtual ~iterator();

            virtual void forEach(const std::function<void(T &)> &fn) const;
            iterator *set(Matrix<T> *matrix) {
                this->matrix = matrix;
                return this;
            }
        };

        class zigzagIterator : public iterator {
        public:
            zigzagIterator(const uint8_t channel);
            virtual ~zigzagIterator();

            void forEach(const std::function<void(T &)> &fn) const override;
        };

        class zOrderIterator : public iterator {
        private:
            bool test() const;
            void iterate(const std::function<void(T &)> &fn, const uint64_t size,
                const uint64_t xOffset, const uint64_t yOffset) const;

        public:
            zOrderIterator(const uint8_t channel);
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
            hilbertIterator(const uint8_t channel);
            virtual ~hilbertIterator();

            void forEach(const std::function<void(T &)> &fn) const override;
        };

        class blockingIterator : public iterator {
        private:
            const uint64_t blockWidth;
            const uint64_t blockHeight;

        public:
            blockingIterator(const uint8_t channel, const uint64_t blockWidth, const uint64_t blockHeight);
            virtual ~blockingIterator();

            void forEach(const std::function<void(T &)> &fn) const override;
        };

        Matrix(const uint64_t width, const uint64_t height, const uint8_t channels = 1U);
        Matrix(const Matrix<T> &matrix);
        virtual ~Matrix();

        template<typename Iterator>
        Matrix<T> *load(Iterator first, Iterator last);
        Matrix<T> *release();

        T at(const int64_t x, const int64_t y, const uint8_t ch,
            const Matrix<T>::Boundary boundary = Matrix<T>::Boundary::ZERO_SHIFT) const;
        T &operator[](const std::tuple<uint64_t, uint64_t, uint8_t> &pos);
        const T &operator[](const std::tuple<uint64_t, uint64_t, uint8_t> &pos) const;

        Matrix<T> *shift(const int x, const int y, const Matrix<T>::Boundary boundary) const;

        uint64_t getWidth() const {
            return this->width;
        }

        uint64_t getHeight() const {
            return this->height;
        }

        uint8_t getChannels() const {
            return this->channels;
        }

        std::vector<T> &getData() {
            return this->data;
        }

        const std::vector<T> &getData() const {
            return this->data;
        }
    };

    template<typename T>
    Matrix<T>::Matrix(const uint64_t width, const uint64_t height, const uint8_t channels) :
            width(width), height(height), channels(channels) {
        this->data.resize(height * width * channels, Matrix<T>::ZERO);
    }

    template<typename T>
    Matrix<T>::Matrix(const Matrix<T> &matrix) :
            width(matrix.width), height(matrix.height), channels(matrix.channels) {
        this->load(matrix.getData().cbegin(), matrix.getData().cend());
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
        this->data.shrink_to_fit();
        return this;
    }

    template<typename T>
    T
    Matrix<T>::at(const int64_t x, const int64_t y, const uint8_t ch,
            const Matrix<T>::Boundary boundary) const {
        if (ch >= this->channels) {
            return T(0);
        } else if (x >= 0 && y >= 0 && static_cast<uint64_t>(x) < this->height &&
                static_cast<uint64_t>(y) < this->width) {
            return this->data[x * this->width * this->channels + y * this->channels + ch];
        }  else if (boundary == Matrix::Boundary::ODD_SYMMETERIC) {
            uint64_t _x = std::abs(x);
            _x %= 2 * this->height - 1;
            _x = _x < this->height ? _x : 2 * this->height - 2 - _x;
            uint64_t _y = std::abs(y);
            _y %= 2 * this->width - 1;
            _y = _y < this->width ? _y : 2 * this->width - 2 - _y;
            return this->data[_x * this->width * this->channels + _y * this->channels + ch];
        } else if (boundary == Matrix::Boundary::REPLICATION) {
            uint64_t _x = std::max(x, 0L);
            _x = std::min(_x, this->height - 1);
            uint64_t _y = std::max(y, 0L);
            _y = std::min(_y, this->width - 1);
            return this->data[_x * this->width * this->channels + _y * this->channels + ch];
        }
        return T(0);
    }

    template<typename T>
    T &
    Matrix<T>::operator[](const std::tuple<uint64_t, uint64_t, uint8_t> &pos) {
        const auto &[x, y, ch] = pos;
        if (x >= this->height || y >= this->width || ch >= this->channels) {
            return *new T(Matrix<T>::ZERO);
        }
        return this->data[x * this->width * this->channels + y * this->channels + ch];
    }

    template<typename T>
    const T &
    Matrix<T>::operator[](const std::tuple<uint64_t, uint64_t, uint8_t> &pos) const {
        const auto &[x, y, ch] = pos;
        if (x >= this->height || y >= this->width || ch >= this->channels) {
            return *new T(Matrix<T>::ZERO);
        }
        return this->data[x * this->width * this->channels + y * this->channels + ch];
    }

    template<typename T>
    Matrix<T> *
    Matrix<T>::shift(const int x, const int y, const Matrix<T>::Boundary boundary) const {
        auto matrix = new Matrix<T>(this->width, this->height, this->channels);
        for (auto ch = 0U; ch < this->channels; ++ch) {
            for (auto i = 0UL; i < this->height; ++i) {
                for (auto j = 0UL; j < this->width; ++j) {
                    (*matrix)[std::make_tuple(i, j, ch)] = this->at(i - x, j - y, ch, boundary);
                }
            }
        }
        return matrix;
    }

    template<typename T>
    Matrix<T>::iterator::iterator(const uint8_t channel) : iterator(channel) {}

    template<typename T>
    Matrix<T>::iterator::~iterator() {}

    template<typename T>
    void
    Matrix<T>::iterator::forEach(const std::function<void(T &)> &fn) const {
        for (auto i = 0UL; i < this->matrix->height; ++i) {
            for (auto j = 0UL; j < this->matrix->width; ++j) {
                fn((*this->matrix)[std::make_tuple(i, j, this->channel)]);
            }
        }
    }

    template<typename T>
    Matrix<T>::zigzagIterator::zigzagIterator(const uint8_t channel) : iterator(channel) {}

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
    Matrix<T>::zOrderIterator::zOrderIterator(const uint8_t channel) : iterator(channel) {}

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
            fn((*this->matrix)[std::make_tuple(xOffset, yOffset, this->channel)]);
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
    Matrix<T>::hilbertIterator::hilbertIterator(const uint8_t channel) : iterator(channel) {}

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
            fn((*this->matrix)[std::make_tuple(xOffset, yOffset, this->channel)]);
            return;
        }
        const uint64_t nextSize = size >> 1;
        this->iterate(fn, nextSize, xOffset, yOffset, x2 >> 1, y2 >> 1, x1 >> 1, y1 >> 1);
        this->iterate(fn, nextSize, xOffset + x1, yOffset + y1, x1 >> 1, y1 >> 1, x2 >> 1, y2 >> 1);
        this->iterate(fn, nextSize, xOffset + x1 + x2, yOffset + y1 + y2, x1 >> 1, y1 >> 1, x2 >> 1, y2 >> 1);
        this->iterate(fn, nextSize, xOffset + x1, yOffset + y1, -x2 >> 1, -y2 >> 1, -x1 >> 1, -y1 >> 1);
    }

    template<typename T>
    Matrix<T>::blockingIterator::blockingIterator(const uint8_t channel,
                const uint64_t blockWidth, const uint64_t blockHeight) :
        iterator(channel), blockWidth(blockWidth), blockHeight(blockHeight) {}

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
                        fn((*this->matrix)[std::make_tuple(i + k, j + l, this->channel)]);
                    }
                }
            }
        }
    }
}

#endif
