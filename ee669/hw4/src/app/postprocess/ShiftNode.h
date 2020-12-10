#ifndef __EE669_SHIFT_NODE
#define __EE669_SHIFT_NODE

#include <string>

#include "common/Matrix.h"
#include "graph/Node.h"

namespace ee669 {
    class ShiftNode : public Node<ee669::Matrix<uint8_t>> {
    private:
        const std::string inputNode;
        const int64_t shiftX;
        const int64_t shiftY;
        const Matrix<uint8_t>::Boundary boundary;

    public:
        ShiftNode(const std::string &key, const std::string &inputNode,
            const int64_t shiftX, const int64_t shiftY, const Matrix<uint8_t>::Boundary boundary);
        virtual ~ShiftNode();

        ee669::Matrix<uint8_t> *eval(Session *session) const override;
    };
}

#endif