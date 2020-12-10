#include <string>
#include <cmath>

#include "LowpassFilterNode.h"
#include "common/Matrix.h"
#include "graph/Node.h"

using ee669::LowpassFilterNode;

LowpassFilterNode::LowpassFilterNode(const std::string &key, const std::string &inputNode,
        const ee669::Matrix<uint8_t>::Boundary boundary) :
    Node<ee669::Matrix<uint8_t>>(key), inputNode(inputNode), boundary(boundary) {}

LowpassFilterNode::~LowpassFilterNode() {}

ee669::Matrix<uint8_t> *
LowpassFilterNode::eval(Session *session) const {
    auto input = this->template getDependency<ee669::Matrix<uint8_t>>(session, inputNode);
    auto temp = new ee669::Matrix<uint8_t>(*input);
    for (auto i = 0UL; i < input->getHeight(); ++i) {
        for (auto j = LowpassFilterNode::blockSize; j < input->getWidth(); j += LowpassFilterNode::blockSize) {
            for (auto ch = 0U; ch < input->getChannels(); ++ch) {
                const int32_t p0 = (*input)[std::make_tuple(i, j - 1, ch)];
                const int32_t q0 = (*input)[std::make_tuple(i, j, ch)];
                if (std::abs(q0 - p0) < 4) {
                    continue;
                }
                const int32_t p1 = (*input)[std::make_tuple(i, j - 2, ch)];
                const int32_t q1 = (*input)[std::make_tuple(i, j + 1, ch)];
                const int32_t p2 = (*input)[std::make_tuple(i, j - 3, ch)];
                const int32_t q2 = (*input)[std::make_tuple(i, j + 2, ch)];
                auto d = (4 * (q0 - p0) - 2 * (q1 - p1) + (q2 - p2) + 4) / 8;
                d = std::min(std::max(d, -24), 24);
                const auto newP0 = std::min(std::max(p0 + d, 0x00), 0xff);
                const auto newQ0 = std::min(std::max(q0 - d, 0x00), 0xff);
                (*temp)[std::make_tuple(i, j - 1, ch)] = newP0;
                (*temp)[std::make_tuple(i, j, ch)] = newQ0;
            }
        }
    }
    auto matrix = new ee669::Matrix<uint8_t>(*temp);
    for (auto i = LowpassFilterNode::blockSize; i < input->getHeight(); i += LowpassFilterNode::blockSize) {
        for (auto j = 0UL; j < input->getWidth(); ++j) {
            for (auto ch = 0U; ch < input->getChannels(); ++ch) {
                const int32_t p0 = (*temp)[std::make_tuple(i - 1, j, ch)];
                const int32_t q0 = (*temp)[std::make_tuple(i, j, ch)];
                if (std::abs(q0 - p0) < 4) {
                    continue;
                }
                const int32_t p1 = (*temp)[std::make_tuple(i - 2, j, ch)];
                const int32_t q1 = (*temp)[std::make_tuple(i + 1, j, ch)];
                const int32_t p2 = (*temp)[std::make_tuple(i - 3, j, ch)];
                const int32_t q2 = (*temp)[std::make_tuple(i + 2, j, ch)];
                auto d = (4 * (q0 - p0) - 2 * (q1 - p1) + (q2 - p2) + 4) / 8;
                d = std::min(std::max(d, -24), 24);
                const auto newP0 = std::min(std::max(p0 + d, 0x00), 0xff);
                const auto newQ0 = std::min(std::max(q0 - d, 0x00), 0xff);
                (*matrix)[std::make_tuple(i - 1, j, ch)] = newP0;
                (*matrix)[std::make_tuple(i, j, ch)] = newQ0;
            }
        }
    }
    delete temp;
    return matrix;
}
