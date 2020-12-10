#include <vector>

#include "JPEGEncodingGraph.h"
#include "graph/Graph.h"
#include "graph/Node.h"
#include "graph/SerializationNode.h"
#include "jpeg-lib/DecodingNode.h"
#include "jpeg-lib/JPEGEncodingNode.h"

using ee669::JPEGEncodingGraph;

JPEGEncodingGraph::JPEGEncodingGraph(const std::string &inputKey, const std::string &decKey,
        const std::string &encKey, const int quality) :
    inputKey(inputKey), decKey(decKey), encKey(encKey), quality(quality) {}

JPEGEncodingGraph::~JPEGEncodingGraph() {}

ee669::Node<std::vector<uint8_t>> *
JPEGEncodingGraph::createGraph() {
    auto dec = this->addNode(new ee669::DecodingNode(this->decKey, this->inputKey));

    auto enc = this->addNode(new ee669::JPEGEncodingNode(this->encKey, this->decKey, this->quality))
        ->dependOn(dec);

    const auto decJPEGKey = "dec_jpeg";
    auto decJPEG = this->addNode(new ee669::DecodingNode(decJPEGKey, this->encKey))
        ->dependOn(enc);

    return this->addNode(new ee669::SerializationNode<uint8_t>("result", decJPEGKey))
        ->dependOn(decJPEG);
}
