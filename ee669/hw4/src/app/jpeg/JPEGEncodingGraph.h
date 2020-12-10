#ifndef __EE669_JPEG_ENCODING_GRAPH
#define __EE669_JPEG_ENCODING_GRAPH

#include <vector>

#include "graph/Graph.h"
#include "graph/Node.h"

namespace ee669 {
    class JPEGEncodingGraph : public ee669::Graph<std::vector<uint8_t>> {
    private:
        const std::string inputKey;
        const std::string decKey;
        const std::string encKey;
        const int quality;

    public:
        JPEGEncodingGraph(const std::string &inputKey, const std::string &decKey,
            const std::string &encKey, const int quality);
        virtual ~JPEGEncodingGraph();

        ee669::Node<std::vector<uint8_t>> *createGraph() override;
    };
}

#endif
