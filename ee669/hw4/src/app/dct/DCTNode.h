#ifndef __EE669_DCT_PROCESS
#define __EE669_DCT_PROCESS

#include <vector>
#include <array>
#include <string>

#include "graph/Node.h"

namespace ee669 {
    class DCTNode : public Node<std::vector<uint8_t>> {
    private:
        static std::array<std::array<uint64_t, 8UL>, 8UL> Q_50;

        const std::string inputNode;
        const uint64_t width;
        const uint64_t height;
        const uint64_t blockSize;
        const uint8_t qualityFactor;

    public:
        DCTNode(const std::string &key, const std::string &inputNode,
            const uint64_t width, const uint64_t height,
            const uint64_t blockSize, const uint8_t qualityLevel);
        virtual ~DCTNode();

        std::vector<uint8_t> *eval(Session *session) const override;
    };
}

#endif
