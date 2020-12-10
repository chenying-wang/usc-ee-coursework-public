#!/bin/bash

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export LD_LIBRARY_PATH="${PWD}/build/lib:${LD_LIBRARY_PATH}"

VMAF_PATH="/home/chenying/vmaf/vmaf-1.5.3"
INPUT_PATH="${PWD}/data"
RESULT_PATH="${PWD}/out"
mkdir -p ${RESULT_PATH}

STAT_BIN="./bin/stat"
STAT_SET1_IMAGES=("house_distor1.bmp" "house_distor2.bmp" "house_distor3.bmp" "house_distor4.bmp" "house_distor5.bmp")
STAT_SET1_ORI_IMAGE="house.bmp"
STAT_SET3_IMAGES=("buildings" "airplane" "boat")

VMAF_BIN="${VMAF_PATH}/libvmaf/build/tools/vmafossexec"
VMAF_SET1_REF="BirdsInCage_30fps"
VMAF_SET1_DIS=("BirdsInCage_40_288_375" "BirdsInCage_80_480_750" "BirdsInCage_95_1080_3000")
VMAF_SET2_REF="CrowdRun_25fps"
VMAF_SET2_DIS=("CrowdRun_03_288_375" "CrowdRun_50_1080_4300" "CrowdRun_90_1080_15000")

for img in ${STAT_SET1_IMAGES[*]}; do
    ${STAT_BIN} -i ${INPUT_PATH}/image_sets_1/${img} --ori ${INPUT_PATH}/image_sets_1/${STAT_SET1_ORI_IMAGE} > /dev/null
done

for img in ${STAT_SET3_IMAGES[*]}; do
    ${STAT_BIN} -o ${RESULT_PATH}/${img}_abs_err.bin -i ${INPUT_PATH}/image_sets_3/${img}.jpg \
        --ori ${INPUT_PATH}/image_sets_3/${img}_ori.bmp
done

ffmpeg -loglevel error -y -f rawvideo -pixel_format gray -video_size 256x256 \
    -i ${RESULT_PATH}/airplane_abs_err.bin ${RESULT_PATH}/airplane_abs_err.png

ffmpeg -loglevel error -y -f rawvideo -pixel_format gray -video_size 512x512 \
    -i ${RESULT_PATH}/boat_abs_err.bin ${RESULT_PATH}/boat_abs_err.png

ffmpeg -loglevel error -y -f rawvideo -pixel_format gray -video_size 768x512 \
    -i ${RESULT_PATH}/buildings_abs_err.bin ${RESULT_PATH}/buildings_abs_err.png

for dis in ${VMAF_SET1_DIS[*]}; do
    ${VMAF_BIN} yuv420p 1920 1080 \
        ${INPUT_PATH}/ref/${VMAF_SET1_REF}.yuv \
        ${INPUT_PATH}/dis/${dis}.yuv \
        ${VMAF_PATH}/model/vmaf_v0.6.1.pkl \
        --psnr --ssim \
        --log ${RESULT_PATH}/${dis}_vmaf_output.xml
done

for dis in ${VMAF_SET2_DIS[*]}; do
    ${VMAF_BIN} yuv420p 1920 1080 \
        ${INPUT_PATH}/ref/${VMAF_SET2_REF}.yuv \
        ${INPUT_PATH}/dis/${dis}.yuv \
        ${VMAF_PATH}/model/vmaf_v0.6.1.pkl \
        --psnr --ssim \
        --log ${RESULT_PATH}/${dis}_vmaf_output.xml
done
