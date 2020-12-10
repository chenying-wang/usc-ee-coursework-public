#ifndef __EE669_STAT_NODE
#define __EE669_STAT_NODE

#include <vector>
#include <string>

#include "common/Matrix.h"
#include "graph/Node.h"

namespace ee669 {
    class StatNode : public Node<std::vector<uint8_t>> {
    private:
        const std::string inputNode;
        const std::string oriNode;
        const std::string mseKey;
        const std::string psnrKey;

    public:
        StatNode(const std::string &key, const std::string &inputNode, const std::string &oriNode,
            const std::string &mseKey, const std::string &psnrKey);
        virtual ~StatNode();

        std::vector<uint8_t> *eval(Session *session) const override;
    };
}

#endif
