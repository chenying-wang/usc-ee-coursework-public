#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "DecodingNode.h"
#include "common/Matrix.h"
#include "graph/Node.h"

using ee669::DecodingNode;

DecodingNode::DecodingNode(const std::string &key, const std::string &inputNode) :
    ee669::Node<ee669::Matrix<uint8_t>>(key), inputNode(inputNode) {}

DecodingNode::~DecodingNode() {}

ee669::Matrix<uint8_t> *
DecodingNode::eval(ee669::Session *session) const {
    auto input = this->template getDependency<std::vector<uint8_t>>(session, this->inputNode);
    auto mat = cv::imdecode(*input, cv::ImreadModes::IMREAD_UNCHANGED);
    if (mat.channels() == 3) {
        cv::cvtColor(mat, mat, cv::ColorConversionCodes::COLOR_BGR2GRAY);
    }
    auto matrix = (new ee669::Matrix<uint8_t>(mat.cols, mat.rows))->release();
    for (auto i = 0; i < mat.rows; ++i) {
        auto p = mat.ptr<uint8_t>(i);
        matrix->getData().insert(matrix->getData().end(), p, p + mat.cols);
    }
    mat.release();
    return matrix;
}
