#include <vector>
#include <string>

#include "StatNode.h"
#include "common/Utils.h"

using ee669::StatNode;

StatNode::StatNode(const std::string &key, const std::string &inputNode, const std::string &oriNode,
        const std::string &mseKey, const std::string &psnrKey) :
    ee669::Node<std::vector<uint8_t>>(key), inputNode(inputNode), oriNode(oriNode), mseKey(mseKey), psnrKey(psnrKey) {}

StatNode::~StatNode() {}

std::vector<uint8_t> *
StatNode::eval(ee669::Session *session) const {
    auto input = this->template getDependency<ee669::Matrix<uint8_t>>(session, this->inputNode);
    auto ori = this->template getDependency<ee669::Matrix<uint8_t>>(session, this->oriNode);
    if (input->getData().size() != ori->getData().size()) {
        session->feed(this->mseKey, new double(0.0))
            ->feed(this->psnrKey, new double(0.0));
        return new std::vector<uint8_t>();
    }

    const auto [psnr, mse] = ee669::Utils::calcPSNR(input->getData().cbegin(), input->getData().cend(),
        ori->getData().cbegin(), ori->getData().cend());
    session->feed(this->mseKey, new double(mse))
        ->feed(this->psnrKey, new double(psnr));

    auto error = new std::vector<uint8_t>(input->getData().size());
    for (auto i = 0UL; i < input->getData().size(); ++i) {
        double r = std::abs(static_cast<int>(input->getData()[i]) - static_cast<int>(ori->getData()[i]));
        r = 1 - r / std::numeric_limits<uint8_t>::max();
        r = std::pow(r, 10);
        (*error)[i] = 0.5 + r * std::numeric_limits<uint8_t>::max();
    }
    return error;
}
