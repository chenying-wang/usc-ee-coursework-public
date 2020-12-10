#include <vector>
#include <string>
#include <cstdio>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "JPEGEncodingNode.h"
#include "common/Matrix.h"
#include "graph/Node.h"
#include "jpeg-6b-lib/jpeglib.h"

using ee669::JPEGEncodingNode;

JPEGEncodingNode::JPEGEncodingNode(const std::string &key, const std::string &inputNode, const int quality):
    ee669::Node<std::vector<uint8_t>>(key), inputNode(inputNode), quality(quality) {}

JPEGEncodingNode::~JPEGEncodingNode() {}

std::vector<uint8_t> *
JPEGEncodingNode::eval(Session *session) const {
    auto input = this->template getDependency<ee669::Matrix<uint8_t>>(session, this->inputNode);
    const auto channels = input->getChannels();
    J_COLOR_SPACE colorSpace = JCS_UNKNOWN;
    if (channels == 1) {
        colorSpace = JCS_GRAYSCALE;
    } else if (channels == 3) {
        colorSpace = JCS_RGB;
    }
    return this->jpegEncode(input->getData().data(), input->getWidth(), input->getHeight(),
        channels, colorSpace, this->quality);
}

std::vector<uint8_t> *
JPEGEncodingNode::jpegEncode(JSAMPLE *image_buffer, const JDIMENSION width, const JDIMENSION height,
        const int channels, const J_COLOR_SPACE colorSpace, const int quality) const {
    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    FILE * outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    char *buffer = nullptr;
    size_t bufferSize = 0UL;

    /* Step 1: allocate and initialize JPEG compression object */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    if ((outfile = open_memstream(&buffer, &bufferSize)) == NULL) {
        throw std::runtime_error("open_memstream() failed");
    }
    jpeg_stdio_dest(&cinfo, outfile);

    /* Step 3: set parameters for compression */
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = channels;
    cinfo.in_color_space = colorSpace;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, true);

    /* Step 4: Start compressor */
    jpeg_start_compress(&cinfo, true);

    /* Step 5: while (scan lines remain to be written) */
    row_stride = width * channels;
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);
    auto compressed = new std::vector<uint8_t>(buffer, buffer + bufferSize);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);

    return compressed;
}

