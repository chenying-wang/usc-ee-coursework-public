#ifndef __EE669_JPEG_ENCODING_NODE
#define __EE669_JPEG_ENCODING_NODE

#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "common/Matrix.h"
#include "graph/Node.h"
#include "jpeg-6b-lib/jpeglib.h"

namespace ee669 {
    class JPEGEncodingNode : public Node<std::vector<uint8_t>> {
    private:
        const std::string inputNode;
        const int quality;

        std::vector<uint8_t> *jpegEncode(JSAMPLE *image_buffer, const JDIMENSION width, const JDIMENSION height,
            const int components, const J_COLOR_SPACE colorSpace, const int quality) const;

    public:
        JPEGEncodingNode(const std::string &key, const std::string &inputNode, const int quality);
        virtual ~JPEGEncodingNode();

        std::vector<uint8_t> *eval(Session *session) const override;
    };
}

#endif
