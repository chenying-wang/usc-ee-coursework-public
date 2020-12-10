#ifndef __EE669_DCT_PROCESS
#define __EE669_DCT_PROCESS

#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>

#include "graph/Node.h"

namespace ee669 {
    class EncoderNode : public Node<std::vector<uint8_t>> {
    private:
        static std::unordered_map<std::string, int> ME_MAP;

        const std::string inputNode;
        const std::string psnrKey;
        const std::string timeKey;
        const uint32_t width;
        const uint32_t height;
        const std::string me;
        const bool noEMV;
        const bool noETRA;

        std::tuple<std::vector<uint8_t> *, double, double> encode(const std::vector<uint8_t> *input) const;

    public:
        EncoderNode(const std::string &key, const std::string &inputNode,
            const std::string &psnrKey, const std::string &timeKey,
            const uint32_t width, const uint32_t height, const std::string &me,
            const bool noEMV, const bool noETRA);
        virtual ~EncoderNode();

        std::vector<uint8_t> *eval(Session *session) const override;
    };
}

#endif
