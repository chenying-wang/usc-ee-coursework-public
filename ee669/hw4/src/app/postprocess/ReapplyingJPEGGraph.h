#ifndef __EE669_REAPPLYING_JPEG_GRAPH
#define __EE669_REAPPLYING_JPEG_GRAPH

#include <vector>
#include <array>
#include <string>

#include "common/Matrix.h"
#include "graph/Graph.h"
#include "graph/Node.h"

namespace ee669 {
    class ReapplyingJPEGGraph : public ee669::Graph<std::vector<uint8_t>> {
    private:
        static std::array<std::array<int64_t, 8UL>, 2UL> shifts;

        const std::string inputKey;
        const std::string decKey;
        const ee669::Matrix<uint8_t>::Boundary boundary;
        const int quality;

    public:
        ReapplyingJPEGGraph(const std::string &inputKey, const std::string &decKey,
            const ee669::Matrix<uint8_t>::Boundary boundary, const int quality);
        virtual ~ReapplyingJPEGGraph();

        ee669::Node<std::vector<uint8_t>> *createGraph() override;
    };
}

#endif
