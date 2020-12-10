#include <vector>
#include <string>

#include "StatGraph.h"
#include "DecodingNode.h"
#include "StatNode.h"

using ee669::StatGraph;

StatGraph::StatGraph(const std::string &inputKey, const std::string &oriKey,
        const std::string &mseKey, const std::string &psnrKey) :
    inputKey(inputKey), oriKey(oriKey), mseKey(mseKey), psnrKey(psnrKey) {}

StatGraph::~StatGraph() {}

ee669::Node<std::vector<uint8_t>> *
StatGraph::createGraph() {
    const auto decKey = "dec", decOriKey = "dec_ori";
    auto dec = this->addNode(new ee669::DecodingNode(decKey, this->inputKey));
    auto decOri = this->addNode(new ee669::DecodingNode(decOriKey, this->oriKey));

    const auto statKey = "stat";
    return this->addNode(new ee669::StatNode(statKey, decKey, decOriKey, this->mseKey, this->psnrKey))
        ->dependOn(dec)
        ->dependOn(decOri);
}
