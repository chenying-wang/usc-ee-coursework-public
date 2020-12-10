#include <vector>
#include <string>
#include <algorithm>

#include "EncoderNode.h"
#define EE669_X264_ME
#include "x264/x264.h"
#undef EE669_X264_ME

using ee669::EncoderNode;

std::unordered_map<std::string, int> ee669::EncoderNode::ME_MAP = {
    {"dia", X264_ME_DIA},
    {"hex", X264_ME_HEX},
    {"umh", X264_ME_UMH},
    {"esa", X264_ME_ESA}
};

EncoderNode::EncoderNode(const std::string &key, const std::string &inputNode,
        const std::string &psnrKey, const std::string &timeKey,
        const uint32_t width, const uint32_t height, const std::string &me,
        const bool noEMV, const bool noETRA) :
    ee669::Node<std::vector<uint8_t>>(key), inputNode(inputNode), psnrKey(psnrKey), timeKey(timeKey),
    width(width), height(height), me(me), noEMV(noEMV), noETRA(noETRA) {}

EncoderNode::~EncoderNode() {}

std::vector<uint8_t> *
EncoderNode::eval(Session *session) const {
    auto input = this->template getDependency<std::vector<uint8_t>>(session, this->inputNode);
    auto [result, psnr, time] = this->encode(input);
    session->feed(this->psnrKey, new double(psnr));
    session->feed(this->timeKey, new double(time));
    return result;
}

std::tuple<std::vector<uint8_t> *, double, double>
EncoderNode::encode(const std::vector<uint8_t> *input) const {
    const int64_t luma_size = this->width * this->height, chroma_size = luma_size / 4;
    const int64_t total_size = luma_size + chroma_size * 2;
    auto result = new std::vector<uint8_t>();
    if (input->empty() || input->size() % total_size) {
        return std::make_tuple(result, 0.0, 0.0);
    }
    const int64_t n_frames = input->size() / total_size;

    x264_param_t param;
    /* Get default params for preset/tuning */
    if (x264_param_default_preset(&param, "medium", NULL) < 0) {
        return std::make_tuple(result, 0.0, 0.0);
    }

    /* Configure non-default params */
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_I420;
    param.i_width  = this->width;
    param.i_height = this->height;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;
    param.b_annexb = 1;
    param.i_threads = 1;
    if (EncoderNode::ME_MAP.count(this->me)) {
        param.analyse.i_me_method = EncoderNode::ME_MAP.at(this->me);
    }
    param.b_no_emv = this->noEMV;
    param.b_no_etra = this->noETRA;
    param.analyse.b_psnr = 1;
    param.analyse.b_psy = 1;

    /* Apply profile restrictions. */
    x264_picture_t pic, pic_out;
    if (x264_param_apply_profile(&param, "high") < 0) {
        return std::make_tuple(result, 0.0, 0.0);
    } else if (x264_picture_alloc(&pic, param.i_csp, param.i_width, param.i_height) < 0) {
        return std::make_tuple(result, 0.0, 0.0);
    }

    x264_t *h = x264_encoder_open(&param);
    if (!(h = x264_encoder_open(&param))) {
        x264_picture_clean(&pic);
        return std::make_tuple(result, 0.0, 0.0);
    }

    /* Encode frames */
    x264_nal_t *nal;
    int i_nal;
    int64_t i_frame = 0, i_frame_size = 0;
    uint64_t offset = 0UL;
    double psnr = 0.0, meTime = 0.0;
    while (i_frame < n_frames) {
        /* Read input frame */
        std::copy(input->data() + offset, input->data() + offset + luma_size, pic.img.plane[0]);
        offset += luma_size;
        std::copy(input->data() + offset, input->data() + offset + chroma_size, pic.img.plane[1]);
        offset += chroma_size;
        std::copy(input->data() + offset, input->data() + offset + chroma_size, pic.img.plane[2]);
        offset += chroma_size;

        pic.i_pts = i_frame++;
        i_frame_size = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
        psnr += pic_out.prop.f_psnr_avg;
        meTime += pic_out.f_me_time;
        if (!i_frame_size) {
            continue;
        } else if (i_frame_size < 0) {
            x264_encoder_close(h);
            x264_picture_clean(&pic);
            return std::make_tuple(result, 0.0, 0.0);
        }
        result->insert(result->end(), nal->p_payload, nal->p_payload + i_frame_size);
    }

    /* Flush delayed frames */
    while(x264_encoder_delayed_frames(h)) {
        i_frame_size = x264_encoder_encode(h, &nal, &i_nal, NULL, &pic_out);
        psnr += pic_out.prop.f_psnr_avg;
        meTime += pic_out.f_me_time;
        if (!i_frame_size) {
            break;
        } else if (i_frame_size < 0) {
            x264_encoder_close(h);
            x264_picture_clean(&pic);
            return std::make_tuple(result, 0.0, 0.0);
        }
        result->insert(result->end(), nal->p_payload, nal->p_payload + i_frame_size);
    }

    psnr /= n_frames;

    x264_encoder_close( h );
    x264_picture_clean( &pic );
    return std::make_tuple(result, psnr, meTime);
}
