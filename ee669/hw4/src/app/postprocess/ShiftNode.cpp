#include <string>

#include "ShiftNode.h"
#include "common/Matrix.h"
#include "graph/Node.h"

using ee669::ShiftNode;

ShiftNode::ShiftNode(const std::string &key, const std::string &inputNode,
        const int64_t shiftX, const int64_t shiftY, const Matrix<uint8_t>::Boundary boundary) :
    ee669::Node<ee669::Matrix<uint8_t>>(key), inputNode(inputNode),
    shiftX(shiftX), shiftY(shiftY), boundary(boundary) {}

ShiftNode::~ShiftNode() {}

ee669::Matrix<uint8_t> *
ShiftNode::eval(Session *session) const {
    auto input = this->template getDependency<ee669::Matrix<uint8_t>>(session, this->inputNode);
    return input->shift(this->shiftX, this->shiftY, this->boundary);
}
