#!/bin/bash

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export LD_LIBRARY_PATH="${PWD}/build/lib:${LD_LIBRARY_PATH}"

PY3="python3"
FFMPEG="ffmpeg"

HISTOGRAM_BIN="${PWD}/bin/histogram"
QUANTIZER_BIN="${PWD}/bin/quantizer"
BLOCK_BIN="${PWD}/bin/block"
STDVQ_BIN="${PWD}/bin/stdvq"
STDVQE_BIN="${PWD}/bin/stdvqe"
TSVQ_BIN="${PWD}/bin/tsvq"
TSVQE_BIN="${PWD}/bin/tsvqe"

IMAGES_PATH="${PWD}/data"
RESULT_PATH="${PWD}/out"
mkdir -p ${RESULT_PATH}

IMAGES=("hat" "house" "plane" "bike" "boat" "people")
IMAGES_SIZE=(256 256)

TRAINING_IMAGES_PROB1=("hat" "house" "plane")
TRAINING_IMAGES_PROB1=("${TRAINING_IMAGES_PROB1[@]/#/${IMAGES_PATH}/}")
TRAINING_IMAGES_PROB1=("${TRAINING_IMAGES_PROB1[@]/%/.raw}")
QUANTIZER_BITS=(2 4)

TRAINING_IMAGES_PROB2=("bike" "boat" "people")
TEST_IMAGES_PROB2=("hat" "house" "plane")
BLCOK_SIZE_ARRAY=(2 4 8)
STDVQ_CODEBOOK_SIZE_ARRAY=(16 32 64)
TSVQ_BITRATE_ARRAY=(4 5 6)

for img in ${IMAGES[*]}; do
    ${HISTOGRAM_BIN} -o ${RESULT_PATH}/${img}.csv -i ${IMAGES_PATH}/${img}.raw
    ${PY3} ./plot/plot_histogram.py -o ${RESULT_PATH}/${img}.pdf -i ${RESULT_PATH}/${img}.csv
done

for bits in ${QUANTIZER_BITS[*]}; do
    for img in ${IMAGES[*]}; do
        ${QUANTIZER_BIN} -b ${bits} --train ${TRAINING_IMAGES_PROB1[*]} \
            -o ${RESULT_PATH}/${img}_${bits}.bin -i ${IMAGES_PATH}/${img}.raw
    done
done

for bits in ${QUANTIZER_BITS[*]}; do
    for img in ${IMAGES[*]}; do
        ${FFMPEG} -loglevel error -y \
            -f rawvideo -pixel_format gray \
            -video_size 256x256 \
            -i ${RESULT_PATH}/${img}_${bits}.bin ${RESULT_PATH}/${img}_${bits}.png
        ${HISTOGRAM_BIN} -o ${RESULT_PATH}/${img}_${bits}.csv -i ${RESULT_PATH}/${img}_${bits}.bin
        ${PY3} ./plot/plot_histogram.py -o ${RESULT_PATH}/${img}_${bits}.pdf -i ${RESULT_PATH}/${img}_${bits}.csv
    done
done

for block_size in ${BLCOK_SIZE_ARRAY[*]}; do
    cat /dev/null > ${RESULT_PATH}/training_block_${block_size}.bin
    for img in ${TRAINING_IMAGES_PROB2[*]}; do
        ${BLOCK_BIN} --width ${IMAGES_SIZE[0]} --height ${IMAGES_SIZE[1]} \
            --block-width ${block_size} --block-height ${block_size} \
            -i ${IMAGES_PATH}/${img}.raw >> ${RESULT_PATH}/training_block_${block_size}.bin
    done
    for img in ${TEST_IMAGES_PROB2[*]}; do
        ${BLOCK_BIN} --width ${IMAGES_SIZE[0]} --height ${IMAGES_SIZE[1]} \
            --block-width ${block_size} --block-height ${block_size} \
            -o ${RESULT_PATH}/${img}_block_${block_size}.bin -i ${IMAGES_PATH}/${img}.raw
    done
done

for block_size in ${BLCOK_SIZE_ARRAY[*]}; do
    for codebook_size in ${STDVQ_CODEBOOK_SIZE_ARRAY[*]}; do
        ${STDVQ_BIN} -d $((${block_size}*${block_size})) -f ${codebook_size} \
            -c ${RESULT_PATH}/stdvq_codebook_${block_size}_${codebook_size}.bin \
            -t ${RESULT_PATH}/training_block_${block_size}.bin
        for img in ${TEST_IMAGES_PROB2[*]}; do
            ${STDVQE_BIN} -c ${RESULT_PATH}/stdvq_codebook_${block_size}_${codebook_size}.bin \
                -o ${RESULT_PATH}/${img}_stdvq_block_${block_size}_${codebook_size}.bin -i ${RESULT_PATH}/${img}_block_${block_size}.bin
            ${BLOCK_BIN} --reverse --width ${IMAGES_SIZE[0]} --height ${IMAGES_SIZE[1]} \
                --block-width ${block_size} --block-height ${block_size} \
                -o ${RESULT_PATH}/${img}_stdvq_${block_size}_${codebook_size}.bin \
                -i ${RESULT_PATH}/${img}_stdvq_block_${block_size}_${codebook_size}.bin
            ${FFMPEG} -loglevel error -y -f rawvideo -pixel_format gray -video_size 256x256 \
                -i ${RESULT_PATH}/${img}_stdvq_${block_size}_${codebook_size}.bin \
                ${RESULT_PATH}/${img}_stdvq_${block_size}_${codebook_size}.png
        done
    done
done

for block_size in ${BLCOK_SIZE_ARRAY[*]}; do
    for bitrate in ${TSVQ_BITRATE_ARRAY[*]}; do
        ${TSVQ_BIN} -d $((${block_size}*${block_size})) -r ${bitrate} \
            -c ${RESULT_PATH}/tsvq_codebook_${block_size}_${bitrate}.bin \
            -t ${RESULT_PATH}/training_block_${block_size}.bin
        for img in ${TEST_IMAGES_PROB2[*]}; do
            ${TSVQE_BIN} -c ${RESULT_PATH}/tsvq_codebook_${block_size}_${bitrate}.bin \
                -o ${RESULT_PATH}/${img}_tsvq_block_${block_size}_${bitrate}.bin -i ${RESULT_PATH}/${img}_block_${block_size}.bin
            ${BLOCK_BIN} --reverse --width ${IMAGES_SIZE[0]} --height ${IMAGES_SIZE[1]} \
                --block-width ${block_size} --block-height ${block_size} \
                -o ${RESULT_PATH}/${img}_tsvq_${block_size}_${bitrate}.bin \
                -i ${RESULT_PATH}/${img}_tsvq_block_${block_size}_${bitrate}.bin
            ${FFMPEG} -loglevel error -y -f rawvideo -pixel_format gray -video_size 256x256 \
                -i ${RESULT_PATH}/${img}_tsvq_${block_size}_${bitrate}.bin \
                ${RESULT_PATH}/${img}_tsvq_${block_size}_${bitrate}.png
        done
    done
done
