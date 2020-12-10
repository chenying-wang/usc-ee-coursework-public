#!/bin/bash

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export LD_LIBRARY_PATH="${PWD}/build/lib:${LD_LIBRARY_PATH}"

IMAGES_PATH="${PWD}/data"
RESULT_PATH="${PWD}/out"
mkdir -p ${RESULT_PATH}

DCT_BIN="./bin/dct"
DCT_IMAGE="clock"
DCT_IMAGE_SIZE=(16 16)
DCT_QUALITY_ARRAY=(20 50 80)

JPEG_BIN="./bin/jpeg"
JPEG_IMAGE="lena"
JPEG_QUALITY_ARRAY=(100 70 50 30 10 1)

POSTPROCESS_BIN="./bin/postprocess"
POSTPROCESS_BOUNDARY_ARRAY=("symmetric" "replication" "zero")
POSTPROCESS_IMAGES=("airplane" "house")
POSTPROCESS_QF_ARRAY=(10 20 40 60 80)

${DCT_BIN} --width ${DCT_IMAGE_SIZE[0]} --height ${DCT_IMAGE_SIZE[1]} \
    --quality 0 \
    -o ${RESULT_PATH}/${DCT_IMAGE}_dct_coef.bin -i ${IMAGES_PATH}/${DCT_IMAGE}.raw
# hexdump -v -e '16/8 "%.1f " "\n"' ${RESULT_PATH}/${DCT_IMAGE}_dct_coef.bin

for quality in ${DCT_QUALITY_ARRAY[*]}; do
    ${DCT_BIN} --width ${DCT_IMAGE_SIZE[0]} --height ${DCT_IMAGE_SIZE[1]} \
        --quality ${quality} \
        -o ${RESULT_PATH}/${DCT_IMAGE}_dct_${quality}.bin -i ${IMAGES_PATH}/${DCT_IMAGE}.raw
    # hexdump -v -e '16/1 "%d " "\n"' ${RESULT_PATH}/${DCT_IMAGE}_dct_${quality}.bin
done

for quality in ${JPEG_QUALITY_ARRAY[*]}; do
    ${JPEG_BIN} --quality ${quality} \
        -o ${RESULT_PATH}/${JPEG_IMAGE}_${quality}.jpg -i ${IMAGES_PATH}/${JPEG_IMAGE}.bmp
done

for img in ${POSTPROCESS_IMAGES[*]}; do
    for i in {1..5}; do
        ${POSTPROCESS_BIN} --algo lowpass \
            --boundary replication \
            --ori ${IMAGES_PATH}/${img}.raw \
            -o ${RESULT_PATH}/${img}${i}_lowpass.bin \
            -i ${IMAGES_PATH}/${img}${i}.jpg
        for boundary in ${POSTPROCESS_BOUNDARY_ARRAY[*]}; do
            ${POSTPROCESS_BIN} --algo reapply \
                --boundary ${boundary} \
                --quality ${POSTPROCESS_QF_ARRAY[$i]} \
                --ori ${IMAGES_PATH}/${img}.raw \
                -o ${RESULT_PATH}/${img}${i}_${boundary}.bin \
                -i ${IMAGES_PATH}/${img}${i}.jpg
        done
    done
done

for i in {1..5}; do
    ffmpeg -loglevel error -y -f rawvideo -pixel_format gray -video_size 256x256 \
        -i ${RESULT_PATH}/airplane${i}_lowpass.bin ${RESULT_PATH}/airplane${i}_lowpass.png
    ffmpeg -loglevel error -y -f rawvideo -pixel_format rgb24 -video_size 256x256 \
        -i ${RESULT_PATH}/house${i}_lowpass.bin ${RESULT_PATH}/house${i}_lowpass.png
    for boundary in ${POSTPROCESS_BOUNDARY_ARRAY[*]}; do
        ffmpeg -loglevel error -y -f rawvideo -pixel_format gray -video_size 256x256 \
            -i ${RESULT_PATH}/airplane${i}_${boundary}.bin ${RESULT_PATH}/airplane${i}_${boundary}.png
        ffmpeg -loglevel error -y -f rawvideo -pixel_format rgb24 -video_size 256x256 \
            -i ${RESULT_PATH}/house${i}_${boundary}.bin ${RESULT_PATH}/house${i}_${boundary}.png
    done
done
