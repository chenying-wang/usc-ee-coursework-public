#ifndef __EE669_STAT_GRAPH
#define __EE669_STAT_GRAPH

#include <vector>
#include <string>

#include "graph/Graph.h"
#include "graph/Node.h"

namespace ee669 {
    class StatGraph : public ee669::Graph<std::vector<uint8_t>> {
    private:
        const std::string inputKey;
        const std::string oriKey;
        const std::string mseKey;
        const std::string psnrKey;

    public:
        StatGraph(const std::string &inputKey, const std::string &oriKey,
            const std::string &mseKey, const std::string &psnrKey);
        virtual ~StatGraph();

        ee669::Node<std::vector<uint8_t>> *createGraph() override;
    };
}

#endif
