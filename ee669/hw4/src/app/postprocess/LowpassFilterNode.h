#ifndef __EE669_LOWPASS_FILTER_NODE
#define __EE669_LOWPASS_FILTER_NODE

#include <string>

#include "common/Matrix.h"
#include "graph/Node.h"

namespace ee669 {
    class LowpassFilterNode : public Node<ee669::Matrix<uint8_t>> {
    private:
        constexpr static uint64_t blockSize = 8UL;
        const std::string inputNode;
        const ee669::Matrix<uint8_t>::Boundary boundary;

    public:
        LowpassFilterNode(const std::string &key, const std::string &inputNode,
            const ee669::Matrix<uint8_t>::Boundary boundary);
        virtual ~LowpassFilterNode();

        ee669::Matrix<uint8_t> *eval(Session *session) const override;
    };
}

#endif
