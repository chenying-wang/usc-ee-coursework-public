#include <vector>
#include <string>

#include "AverageNode.h"
#include "common/Matrix.h"
#include "graph/Node.h"

using ee669::AverageNode;

AverageNode::AverageNode(const std::string &key, const std::vector<std::string> &inputNodes) :
    ee669::Node<ee669::Matrix<uint8_t>>(key), inputNodes(inputNodes) {}

AverageNode::~AverageNode() {}

ee669::Matrix<uint8_t> *
AverageNode::eval(Session *session) const {
    ee669::Matrix<uint64_t> *sum = nullptr;
    for (const auto &inputNode : this->inputNodes) {
        auto input = this->template getDependency<ee669::Matrix<uint8_t>>(session, inputNode);
        if (sum == nullptr) {
            sum = new ee669::Matrix<uint64_t>(input->getWidth(), input->getHeight(), input->getChannels());
        } else if (sum->getWidth() != input->getWidth() || sum->getHeight() != input->getHeight()) {
            return new ee669::Matrix<uint8_t>(input->getWidth(), input->getHeight(), input->getChannels());
        }
        for (auto ch = 0U; ch < sum->getChannels(); ++ch) {
            for (auto i = 0UL; i < sum->getHeight(); ++i) {
                for (auto j = 0UL; j < sum->getWidth(); ++j) {
                    (*sum)[std::make_tuple(i, j, ch)] += (*input)[std::make_tuple(i, j, ch)];
                }
            }
        }
    }

    const auto size = this->inputNodes.size();
    for (auto ch = 0U; ch < sum->getChannels(); ++ch) {
        for (auto i = 0UL; i < sum->getHeight(); ++i) {
            for (auto j = 0UL; j < sum->getWidth(); ++j) {
                (*sum)[std::make_tuple(i, j, ch)] /= static_cast<double>(size);
            }
        }
    }
    auto avg = new ee669::Matrix<uint8_t>(sum->getWidth(), sum->getHeight(), sum->getChannels());
    avg->load(sum->getData().cbegin(), sum->getData().cend());
    delete sum;
    return avg;
}
