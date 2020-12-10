#ifndef __EE669_DECODING_NODE
#define __EE669_DECODING_NODE

#include <string>

#include "common/Matrix.h"
#include "graph/Node.h"

namespace ee669 {
    class DecodingNode : public Node<ee669::Matrix<uint8_t>> {
    private:
        const std::string inputNode;

    public:
        DecodingNode(const std::string &key, const std::string &inputNode);
        virtual ~DecodingNode();

        ee669::Matrix<uint8_t> *eval(Session *session) const override;
    };
}

#endif
