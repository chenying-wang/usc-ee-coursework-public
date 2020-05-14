#!/bin/bash

echo "
################################################################
# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, February 14, 2020
################################################################
"

CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export LD_LIBRARY_PATH="/usr/local/lib:${CUR_DIR}/build/lib:${LD_LIBRARY_PATH}"

SUCCESS="0"
ERROR="1"

INPUT_IMAGE_PATH="./img"
if [ $# -ge 1 ]; then
    if [ -d $1 ]; then
        INPUT_IMAGE_PATH="$1"
    else
        echo "$1 is not exists"
        exit ${ERROR}
    fi
else
   if [ ! -d ${INPUT_IMAGE_PATH} ]; then
        echo "${INPUT_IMAGE_PATH} is not exists"
        exit ${ERROR}
    fi
fi
OUTPUT_IMAGE_PATH="${INPUT_IMAGE_PATH}/result"
OUTPUT_PATH="${INPUT_IMAGE_PATH}/out"

[ ! -d ${OUTPUT_IMAGE_PATH} ] && mkdir ${OUTPUT_IMAGE_PATH}
[ ! -d ${OUTPUT_PATH} ] && mkdir ${OUTPUT_PATH}

if [ ! -d ${INPUT_IMAGE_PATH} ]; then
    echo "${INPUT_IMAGE_PATH} is not exists"
    exit ${ERROR}
fi

BIN_PATH="${CUR_DIR}/bin"

#### Edge Config ####
EDGE_BIN="${BIN_PATH}/edge"
EDGE_INPUT_GALLERY="${INPUT_IMAGE_PATH}/Gallery.raw"
EDGE_INPUT_DOGS="${INPUT_IMAGE_PATH}/Dogs.raw"
EDGE_INPUT="${INPUT_IMAGE_PATH}/Gallery.raw"
EDGE_WIDTH="481"
EDGE_HEIGHT="321"
EDGE_CHANNEL="3"

#### Halftone Run ####
HALFTONE_BIN="${BIN_PATH}/halftone"
HALFTONE_INPUT_LIGHTHOUSE="${INPUT_IMAGE_PATH}/LightHouse.raw"
HALFTONE_WIDTHT_LIGHTHOUSE="750"
HALFTONE_HEIGHTT_LIGHTHOUSE="500"
HALFTONE_CHANNEL_LIGHTHOUSE="1"
HALFTONE_INPUT_ROSE="${INPUT_IMAGE_PATH}/Rose.raw"
HALFTONE_WIDTH_ROSE="640"
HALFTONE_HEIGHT_ROSE="480"
HALFTONE_CHANNEL_ROSE="3"

echo "Image will be Output to ${OUTPUT_IMAGE_PATH}"
echo "Others will be Output to ${OUTPUT_PATH}"

#### Edge Run ####
echo "Edge Start"
${EDGE_BIN} -a sobel \
    -i ${EDGE_INPUT_GALLERY} \
    -o ${OUTPUT_IMAGE_PATH}/Gallery_sobel.raw \
    -w ${EDGE_WIDTH} -h ${EDGE_HEIGHT} \
    -c ${EDGE_CHANNEL} \
    --threshold 28 \
    --x-gradient-output ${OUTPUT_IMAGE_PATH}/Gallery_x_gradient.raw \
    --y-gradient-output ${OUTPUT_IMAGE_PATH}/Gallery_y_gradient.raw \
    --gradient-output ${OUTPUT_IMAGE_PATH}/Gallery_gradient.raw \
    > ${OUTPUT_PATH}/edge_sobel_Gallery.log

${EDGE_BIN} -a sobel \
    -i ${EDGE_INPUT_DOGS} \
    -o ${OUTPUT_IMAGE_PATH}/Dogs_sobel.raw \
    -w ${EDGE_WIDTH} -h ${EDGE_HEIGHT} \
    -c ${EDGE_CHANNEL} \
    --threshold 52 \
    --x-gradient-output ${OUTPUT_IMAGE_PATH}/Dogs_x_gradient.raw \
    --y-gradient-output ${OUTPUT_IMAGE_PATH}/Dogs_y_gradient.raw \
    --gradient-output ${OUTPUT_IMAGE_PATH}/Dogs_gradient.raw \
    > ${OUTPUT_PATH}/edge_sobel_Dogs.log

${EDGE_BIN} -a canny \
    -i ${EDGE_INPUT_GALLERY} \
    -o ${OUTPUT_IMAGE_PATH}/Gallery_canny.raw \
    -w ${EDGE_WIDTH} -h ${EDGE_HEIGHT} \
    -c ${EDGE_CHANNEL} \
    --threshold 208 \
    --low-threshold 96 \
    > ${OUTPUT_PATH}/edge_canny_Gallery.log

${EDGE_BIN} -a canny \
    -i ${EDGE_INPUT_DOGS} \
    -o ${OUTPUT_IMAGE_PATH}/Dogs_canny.raw \
    -w ${EDGE_WIDTH} -h ${EDGE_HEIGHT} \
    -c ${EDGE_CHANNEL} \
    --threshold 240 \
    --low-threshold 176 \
    > ${OUTPUT_PATH}/edge_canny_Dogs.log
echo "Edge Done"

#### Halftone Run ####
echo "Halftone Start"
${HALFTONE_BIN} -a fixed  \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_fixed.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --threshold 128 \
    > ${OUTPUT_PATH}/halftone_fixed.log

${HALFTONE_BIN} -a random  \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_random.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --threshold 128 \
    > ${OUTPUT_PATH}/halftone_random.log

${HALFTONE_BIN} -a dithering_matrix  \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_dithering_matrix_1.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --dithering-matrix-size 1 \
    > ${OUTPUT_PATH}/halftone_dithering_matrix_1.log

${HALFTONE_BIN} -a dithering_matrix  \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_dithering_matrix_3.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --dithering-matrix-size 3 \
    > ${OUTPUT_PATH}/halftone_dithering_matrix_3.log

${HALFTONE_BIN} -a dithering_matrix  \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_dithering_matrix_5.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --dithering-matrix-size 5 \
    > ${OUTPUT_PATH}/halftone_dithering_matrix_5.log

${HALFTONE_BIN} -a error_diffusion_fs \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_error_diffusion_fs.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --threshold 128 \
    > ${OUTPUT_PATH}/halftone_error_diffusion_fs.log

${HALFTONE_BIN} -a error_diffusion_jjn  \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_error_diffusion_jjn.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --threshold 128 \
    > ${OUTPUT_PATH}/halftone_error_diffusion_jjn.log

${HALFTONE_BIN} -a error_diffusion_stucki  \
    -i ${HALFTONE_INPUT_LIGHTHOUSE} \
    -o ${OUTPUT_IMAGE_PATH}/LightHouse_error_diffusion_stucki.raw \
    -w ${HALFTONE_WIDTHT_LIGHTHOUSE} -h ${HALFTONE_HEIGHTT_LIGHTHOUSE} \
    -c ${HALFTONE_CHANNEL_LIGHTHOUSE} \
    --threshold 128 \
    > ${OUTPUT_PATH}/halftone_error_diffusion_stucki.log

${HALFTONE_BIN} -a error_diffusion_fs  \
    -i ${HALFTONE_INPUT_ROSE} \
    -o ${OUTPUT_IMAGE_PATH}/Rose_error_diffusion_fs.raw \
    -w ${HALFTONE_WIDTH_ROSE} -h ${HALFTONE_HEIGHT_ROSE} \
    -c ${HALFTONE_CHANNEL_ROSE} \
    --threshold 128 \
    > ${OUTPUT_PATH}/halftone_rgb_error_diffusion_fs.log

${HALFTONE_BIN} -a error_diffusion_mbvq_fs  \
    -i ${HALFTONE_INPUT_ROSE} \
    -o ${OUTPUT_IMAGE_PATH}/Rose_error_diffusion_mbvq_fs.raw \
    -w ${HALFTONE_WIDTH_ROSE} -h ${HALFTONE_HEIGHT_ROSE} \
    -c ${HALFTONE_CHANNEL_ROSE} \
    > ${OUTPUT_PATH}/halftone_rgb_error_diffusion_mbvq_fs.log

${HALFTONE_BIN} -a error_diffusion_mbvq_jjn  \
    -i ${HALFTONE_INPUT_ROSE} \
    -o ${OUTPUT_IMAGE_PATH}/Rose_error_diffusion_mbvq_jjn.raw \
    -w ${HALFTONE_WIDTH_ROSE} -h ${HALFTONE_HEIGHT_ROSE} \
    -c ${HALFTONE_CHANNEL_ROSE} \
    > ${OUTPUT_PATH}/halftone_rgb_error_diffusion_mbvq_jjn.log
echo "Halftone Done"

exit ${SUCCESS}
