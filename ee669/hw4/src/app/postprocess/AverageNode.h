#ifndef __EE669_AVERAGE_NODE
#define __EE669_AVERAGE_NODE

#include <vector>
#include <string>

#include "common/Matrix.h"
#include "graph/Node.h"

namespace ee669 {
    class AverageNode : public Node<ee669::Matrix<uint8_t>> {
    private:
        const std::vector<std::string> inputNodes;

    public:
        AverageNode(const std::string &key, const std::vector<std::string> &inputNodes);
        virtual ~AverageNode();

        ee669::Matrix<uint8_t> *eval(Session *session) const override;
    };
}

#endif
