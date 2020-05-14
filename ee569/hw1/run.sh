#!/bin/bash

echo "
################################################################
# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, January 24, 2020
################################################################
"

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

PYTHON3="python3"
BIN_PATH="./bin"

#### Demosacing Config ####
DEMOSAICING_BIN="${BIN_PATH}/demosaic"
DEMOSAICING_INPUT="${INPUT_IMAGE_PATH}/Dog.raw"
DEMOSAICING_ORI="${INPUT_IMAGE_PATH}/Dog_ori.raw"
DEMOSAICING_WIDTH="600"
DEMOSAICING_HEIGHT="532"

#### Histogram Config ####
HISTOGRAM_BIN="${BIN_PATH}/histogram"
HISTOGRAM_INPUT="${INPUT_IMAGE_PATH}/Toy.raw"
HISTOGRAM_WIDTH="400"
HISTOGRAM_HEIGHT="560"
HISTOGRAM_CHANNEL="3"

#### Denoising Config ####
DENOISING_BIN="${BIN_PATH}/denoise"
DENOISING_INPUT="${INPUT_IMAGE_PATH}/Corn_noisy.raw"
DENOISING_ORI="${INPUT_IMAGE_PATH}/Corn_gray.raw"
DENOISING_WIDTH="320"
DENOISING_HEIGHT="320"

echo "Image will be Output to ${OUTPUT_IMAGE_PATH}"
echo "Others will be Output to ${OUTPUT_PATH}"

#### Demosacing Run ####
echo "Demosaic Start"
${DEMOSAICING_BIN} -a bilinear \
    -i ${DEMOSAICING_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Dog_demosaic_bilinear.raw \
    --ori ${DEMOSAICING_ORI} \
    -w ${DEMOSAICING_WIDTH} -h ${DEMOSAICING_HEIGHT} \
    > ${OUTPUT_PATH}/demosaic_blinear.log

${DEMOSAICING_BIN} -a mhc \
    -i ${DEMOSAICING_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Dog_demosaic_mhc.raw \
    --ori ${DEMOSAICING_ORI} \
    -w ${DEMOSAICING_WIDTH} -h ${DEMOSAICING_HEIGHT} \
    > ${OUTPUT_PATH}/demosaic_mhc.log
echo "Demosaic Done"

#### Histogram Run ####
echo "Histogram Start"
${HISTOGRAM_BIN} -a transfer_function \
    -i ${HISTOGRAM_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Toy_histogram_transfer_function.raw \
    -w ${HISTOGRAM_WIDTH} -h ${HISTOGRAM_HEIGHT} \
    -c ${HISTOGRAM_CHANNEL} \
    --ori-pdf-output ${OUTPUT_PATH}/histogram_ori_pdf.csv \
    --transfer-function-output ${OUTPUT_PATH}/histogram_transfer_function.csv \
    > ${OUTPUT_PATH}/histogram_transfer_function.log

${HISTOGRAM_BIN} -a bucket_filling \
    -i ${HISTOGRAM_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Toy_histogram_histogram_bucket_filling.raw \
    -w ${HISTOGRAM_WIDTH} -h ${HISTOGRAM_HEIGHT} \
    -c ${HISTOGRAM_CHANNEL} \
    --cdf-output ${OUTPUT_PATH}/histogram_bucket_filling_cdf.csv \
    > ${OUTPUT_PATH}/histogram_bucket_filling.log
echo "Histogram Done"

#### Denoising Run ####
echo "Denoise Start"
${DENOISING_BIN} -a linear_uniform \
    -i ${DENOISING_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Corn_denoise_uniform.raw \
    --ori ${DENOISING_ORI}  \
    -w ${DENOISING_WIDTH} -h ${DENOISING_HEIGHT} \
    --filter-width 3 --filter-height 3 \
    --analyze-noise-histogram ${OUTPUT_PATH}/denoise_noise_pdf.csv \
    --analyze-diff-histogram ${OUTPUT_PATH}/denoise_diff_pdf.csv \
    > ${OUTPUT_PATH}/denoise_uniform.log

${DENOISING_BIN} -a linear_gaussian \
    -i ${DENOISING_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Corn_denoise_uniform.raw \
    --ori ${DENOISING_ORI}  \
    -w ${DENOISING_WIDTH} -h ${DENOISING_HEIGHT} \
    --filter-width 7 --filter-height 7 \
    --sigma-s 1.00 \
    > ${OUTPUT_PATH}/denoise_gaussian.log

${DENOISING_BIN} -a bilateral \
    -i ${DENOISING_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Corn_denoise_bilateral.raw \
    --ori ${DENOISING_ORI}  \
    -w ${DENOISING_WIDTH} -h ${DENOISING_HEIGHT} \
    --filter-width 7 --filter-height 7 \
    --sigma-s 0.90 \
    --sigma-c 65 \
    > ${OUTPUT_PATH}/denoise_bilateral.log

${DENOISING_BIN} -a nlm \
    -i ${DENOISING_INPUT} \
    -o ${OUTPUT_IMAGE_PATH}/Corn_denoise_nlm.raw \
    --ori ${DENOISING_ORI}  \
    -w ${DENOISING_WIDTH} -h ${DENOISING_HEIGHT} \
    --filter-width 25 --filter-height 25 \
    --neighborhood_width 3 --neighborhood_height 3 \
    --nlm-h 20 --nlm-a 2.2 \
    > ${OUTPUT_PATH}/denoise_nlm.log
echo "Denoise Done"

#### Plot Run ####
echo "Plot Start"
${PYTHON3} ./plot/simple_plot.py \
    ${OUTPUT_PATH}/histogram_ori_pdf.csv \
    ${OUTPUT_PATH}/histogram_ori_pdf.png \
    red green blue

${PYTHON3} ./plot/simple_plot.py \
    ${OUTPUT_PATH}/histogram_transfer_function.csv \
    ${OUTPUT_PATH}/histogram_transfer_function.png \
    red green blue

${PYTHON3} ./plot/simple_plot.py \
    ${OUTPUT_PATH}/histogram_bucket_filling_cdf.csv \
    ${OUTPUT_PATH}/histogram_bucket_filling_cdf.png \
    red green blue

${PYTHON3} ./plot/simple_plot.py \
    ${OUTPUT_PATH}/denoise_noise_pdf.csv \
    ${OUTPUT_PATH}/denoise_noise_pdf.png \
    subtraction

${PYTHON3} ./plot/simple_plot.py \
    ${OUTPUT_PATH}/denoise_diff_pdf.csv \
    ${OUTPUT_PATH}/denoise_diff_pdf.png \
    noise original
echo "Plot Done"

exit ${SUCCESS}
