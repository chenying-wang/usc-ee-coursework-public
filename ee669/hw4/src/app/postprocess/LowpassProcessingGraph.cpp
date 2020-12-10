#include <vector>
#include <array>
#include <string>

#include "LowpassProcessingGraph.h"
#include "LowpassFilterNode.h"
#include "common/Matrix.h"
#include "graph/Graph.h"
#include "graph/Node.h"
#include "graph/SerializationNode.h"
#include "jpeg-lib/DecodingNode.h"

using ee669::LowpassProcessingGraph;

LowpassProcessingGraph::LowpassProcessingGraph(const std::string &inputKey, const std::string &decKey,
            const ee669::Matrix<uint8_t>::Boundary boundary) :
    inputKey(inputKey), decKey(decKey), boundary(boundary) {}

LowpassProcessingGraph::~LowpassProcessingGraph() {}

ee669::Node<std::vector<uint8_t>> *
LowpassProcessingGraph::createGraph() {
    auto dec = this->addNode(new ee669::DecodingNode(this->decKey, this->inputKey));

    const auto lowpassKey = "lowpass";
    auto lowpass = this->addNode(new ee669::LowpassFilterNode(lowpassKey, this->decKey, this->boundary))
        ->dependOn(dec);

    return this->addNode(new ee669::SerializationNode<uint8_t>("result", lowpassKey))
        ->dependOn(lowpass);
}
