#include <vector>
#include <array>
#include <string>

#include "ReapplyingJPEGGraph.h"
#include "AverageNode.h"
#include "ShiftNode.h"
#include "common/Matrix.h"
#include "graph/Graph.h"
#include "graph/Node.h"
#include "graph/SerializationNode.h"
#include "jpeg-lib/DecodingNode.h"
#include "jpeg-lib/JPEGEncodingNode.h"

using ee669::ReapplyingJPEGGraph;

std::array<std::array<int64_t, 8UL>, 2UL>
ReapplyingJPEGGraph::shifts = {{
    {-3L, -2L, -1L, 0L, 1L, 2L, 3L, 4L},
    {-3L, -2L, -1L, 0L, 1L, 2L, 3L, 4L}
}};

ReapplyingJPEGGraph::ReapplyingJPEGGraph(const std::string &inputKey, const std::string &decKey,
        const ee669::Matrix<uint8_t>::Boundary boundary, const int quality) :
    inputKey(inputKey), decKey(decKey), boundary(boundary), quality(quality) {}

ReapplyingJPEGGraph::~ReapplyingJPEGGraph() {}

ee669::Node<std::vector<uint8_t>> *
ReapplyingJPEGGraph::createGraph() {
    auto dec = this->addNode(new ee669::DecodingNode(this->decKey, this->inputKey));
    std::vector<std::string> avgDepKeys;
    std::vector<NodeBase *> avgDeps;
    for (auto i = 0UL; i < this->shifts[0].size(); ++i) {
        for (auto j = 0UL; j < this->shifts[1].size(); ++j) {
            const auto shiftX = shifts[0][i], shiftY = shifts[1][j];
            const auto keySuffix = '_' + std::to_string(shiftX) + '_' + std::to_string(shiftY);

            const auto shiftKey = "shift" + keySuffix;
            auto shift = this->addNode(new ee669::ShiftNode(shiftKey, this->decKey,
                    shiftX, shiftY, this->boundary))
                ->dependOn(dec);

            const auto encKey = "enc" + keySuffix;
            auto enc = this->addNode(new ee669::JPEGEncodingNode(encKey, shiftKey, this->quality))
                ->dependOn(shift);

            const auto decShiftedKey = "dec" + keySuffix;
            auto decShifted = this->addNode(new ee669::DecodingNode(decShiftedKey, encKey))
                ->dependOn(enc);

            const auto shiftBackKey = "shift_back" + keySuffix;
            auto shiftBack = this->addNode(new ee669::ShiftNode(shiftBackKey, decShiftedKey,
                    -shiftX, -shiftY, this->boundary))
                ->dependOn(decShifted);

            avgDepKeys.push_back(shiftBackKey);
            avgDeps.push_back(shiftBack);
        }
    }
    const auto avgKey = "avg";
    auto avg = this->addNode(new ee669::AverageNode(avgKey, avgDepKeys));
    for (const auto avgDep :avgDeps) {
        avg->dependOn(avgDep);
    }
    avgDepKeys.clear();
    avgDeps.clear();

    return this->addNode(new ee669::SerializationNode<uint8_t>("result", avgKey))
        ->dependOn(avg);
}
