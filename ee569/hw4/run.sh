#!/bin/bash

echo "
################################################################
# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, March 20, 2020
################################################################
"

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export LD_LIBRARY_PATH="/usr/local/lib:${PWD}/build/lib:${LD_LIBRARY_PATH}"

INPUT_IMAGE_PATH="./img"
if [ $# -ge 1 ]; then
    if [ -d ${1} ]; then
        INPUT_IMAGE_PATH="${1}"
    else
        echo "${1} is not exists"
        exit ${ERROR}
    fi
else
   if [ ! -d ${INPUT_IMAGE_PATH} ]; then
        echo "${INPUT_IMAGE_PATH} is not exists"
        exit ${ERROR}
    fi
fi

source ${PWD}/config/config.sh ${PWD}

[ ! -d ${OUTPUT_IMAGE_PATH} ] && mkdir ${OUTPUT_IMAGE_PATH}
[ ! -d ${OUTPUT_PATH} ] && mkdir ${OUTPUT_PATH}

if [ ! -d ${INPUT_IMAGE_PATH} ]; then
    echo "${INPUT_IMAGE_PATH} is not exists"
    exit ${ERROR}
fi

echo "Image will be Output to ${OUTPUT_IMAGE_PATH}"
echo "Others will be Output to ${OUTPUT_PATH}"

#### Texture Run ####
if [ ${TEXTURE} -ne 0 ]; then
    echo "Texture Start"
    ${TEXTURE_BIN} -a pca \
        --laws-kernel ${TEXTURE_LAWS_KERNEL} \
        --train ${TEXTURE_TRAINING_DATASET} \
        --test ${TEXTURE_TEST_DATASET} \
        --dataset-width ${TEXTURE_DATASET_WIDTH} \
        --dataset-height ${TEXTURE_DATASET_HEIGHT} \
        --pca ${TEXTURE_PCA} \
        --train-pca-csv ${OUTPUT_PATH}/train_pca.csv \
        --test-pca-csv ${OUTPUT_PATH}/test_pca.csv

    ${TEXTURE_BIN} -a k_means \
        --laws-kernel ${TEXTURE_LAWS_KERNEL} \
        --train ${TEXTURE_TRAINING_DATASET} \
        --test ${TEXTURE_TEST_DATASET} \
        --dataset-width ${TEXTURE_DATASET_WIDTH} \
        --dataset-height ${TEXTURE_DATASET_HEIGHT}

    ${TEXTURE_BIN} -a k_means \
        --laws-kernel ${TEXTURE_LAWS_KERNEL} \
        --train ${TEXTURE_TRAINING_DATASET} \
        --test ${TEXTURE_TEST_DATASET} \
        --dataset-width ${TEXTURE_DATASET_WIDTH} \
        --dataset-height ${TEXTURE_DATASET_HEIGHT} \
        --pca ${TEXTURE_PCA}

    ${TEXTURE_BIN} -a rf \
        --laws-kernel ${TEXTURE_LAWS_KERNEL} \
        --train ${TEXTURE_TRAINING_DATASET} \
        --test ${TEXTURE_TEST_DATASET} \
        --dataset-width ${TEXTURE_DATASET_WIDTH} \
        --dataset-height ${TEXTURE_DATASET_HEIGHT} \
        --pca ${TEXTURE_PCA}

    ${TEXTURE_BIN} -a svm \
        --laws-kernel ${TEXTURE_LAWS_KERNEL} \
        --train ${TEXTURE_TRAINING_DATASET} \
        --test ${TEXTURE_TEST_DATASET} \
        --dataset-width ${TEXTURE_DATASET_WIDTH} \
        --dataset-height ${TEXTURE_DATASET_HEIGHT} \
        --pca ${TEXTURE_PCA}

    # ${TEXTURE_BIN} -a k_means_segment \
    #     -i ${TEXTURE_INPUT} \
    #     -o ${OUTPUT_IMAGE_PATH}/comp_texture.raw \
    #     -w ${TEXTURE_WIDTH} -h ${TEXTURE_HEIGHT} \
    #     -c ${TEXTURE_CHANNEL} \
    #     --laws-kernel ${TEXTURE_LAWS_KERNEL} \
    #     --cluster ${TEXTURE_SEGMENT_CLUSTER} \
    #     --window-size ${TEXTURE_SEGMENT_WINDOW_SIZE}

    # ${TEXTURE_BIN} -a k_means_segment \
    #     -i ${TEXTURE_INPUT} \
    #     -o ${OUTPUT_IMAGE_PATH}/comp_texture_pca_7.raw \
    #     -w ${TEXTURE_WIDTH} -h ${TEXTURE_HEIGHT} \
    #     -c ${TEXTURE_CHANNEL} \
    #     --laws-kernel ${TEXTURE_LAWS_KERNEL_7} \
    #     --cluster ${TEXTURE_SEGMENT_CLUSTER} \
    #     --pca ${TEXTURE_SEGMENT_PCA} \
    #     --window-size ${TEXTURE_SEGMENT_WINDOW_SIZE_7}

    # ${TEXTURE_BIN} -a k_means_segment \
    #     -i ${TEXTURE_INPUT} \
    #     -o ${OUTPUT_IMAGE_PATH}/comp_texture_opt.raw \
    #     -w ${TEXTURE_WIDTH} -h ${TEXTURE_HEIGHT} \
    #     -c ${TEXTURE_CHANNEL} \
    #     --laws-kernel ${TEXTURE_LAWS_KERNEL_7} \
    #     --cluster ${TEXTURE_SEGMENT_CLUSTER} \
    #     --pca ${TEXTURE_SEGMENT_PCA_OPT} \
    #     --window-size ${TEXTURE_SEGMENT_WINDOW_SIZE_7} \
    #     --opt-windows
    echo "Texture Done"
fi

#### Feature Run ####
if [ ${FEATURE} -ne 0 ]; then
    echo "Feature Start"
    ${FEATURE_BIN} -a keypoints \
        --query ${FEATURE_HUSKY_3} \
        -o ${OUTPUT_IMAGE_PATH}/keypoints_husky_3.raw

    ${FEATURE_BIN} -a keypoints \
        --query ${FEATURE_HUSKY_1} \
        -o ${OUTPUT_IMAGE_PATH}/keypoints_husky_1.raw

    ${FEATURE_BIN} -a largest_keypoints \
        --query ${FEATURE_HUSKY_3} \
        -o ${OUTPUT_IMAGE_PATH}/largest_keypoint_husky_3.raw

    ${FEATURE_BIN} -a match_largest_scale \
        --query ${FEATURE_HUSKY_3} \
        --train ${FEATURE_HUSKY_1} \
        -o ${OUTPUT_IMAGE_PATH}/match_largest_scale.raw

    ${FEATURE_BIN} -a match \
        --query ${FEATURE_HUSKY_3} \
        --train ${FEATURE_HUSKY_1} \
        -o ${OUTPUT_IMAGE_PATH}/match_1.raw

    ${FEATURE_BIN} -a match \
        --query ${FEATURE_HUSKY_3} \
        --train ${FEATURE_HUSKY_2} \
        -o ${OUTPUT_IMAGE_PATH}/match_2.raw

    ${FEATURE_BIN} -a match \
        --query ${FEATURE_HUSKY_3} \
        --train ${FEATURE_PUPPY} \
        -o ${OUTPUT_IMAGE_PATH}/match_3.raw

    ${FEATURE_BIN} -a match \
        --query ${FEATURE_HUSKY_1} \
        --train ${FEATURE_PUPPY} \
        -o ${OUTPUT_IMAGE_PATH}/match_4.raw

    ${FEATURE_BIN} -a bag_of_words \
        --query ${FEATURE_HUSKY_1} \
        --output-csv ${OUTPUT_PATH}/bow_husky_1.csv

    ${FEATURE_BIN} -a bag_of_words \
        --query ${FEATURE_HUSKY_2} \
        --output-csv ${OUTPUT_PATH}/bow_husky_2.csv

    ${FEATURE_BIN} -a bag_of_words \
        --query ${FEATURE_HUSKY_3} \
        --output-csv ${OUTPUT_PATH}/bow_husky_3.csv

    ${FEATURE_BIN} -a bag_of_words \
        --query ${FEATURE_PUPPY} \
        --output-csv ${OUTPUT_PATH}/bow_puppy.csv

    ${FEATURE_BIN} -a match_bag_of_words \
        --query ${FEATURE_HUSKY_3} \
        --train ${FEATURE_HUSKY_1} \
        --output-csv ${OUTPUT_PATH}/match_bow_husky_1.csv

    ${FEATURE_BIN} -a match_bag_of_words \
        --query ${FEATURE_HUSKY_3} \
        --train ${FEATURE_HUSKY_2} \
        --output-csv ${OUTPUT_PATH}/match_bow_husky_2.csv

    ${FEATURE_BIN} -a match_bag_of_words \
        --query ${FEATURE_HUSKY_3} \
        --train ${FEATURE_PUPPY} \
        --output-csv ${OUTPUT_PATH}/match_bow_puppy.csv
    echo "Feature Done"
fi

#### Plot Run ####
if [ ${PLOT} -ne 0 ]; then
    echo "Plot Start"
    ${PYTHON3} ./plot/plot_3d.py \
        ${OUTPUT_PATH}/train_pca.csv \
        ${OUTPUT_PATH}/train_pca.png

    ${PYTHON3} ./plot/plot_3d.py \
        ${OUTPUT_PATH}/test_pca.csv \
        ${OUTPUT_PATH}/test_pca.png

    ${PYTHON3} ./plot/plot_bar.py \
        ${OUTPUT_PATH}/bow_husky_1.csv \
        ${OUTPUT_PATH}/bow_husky_1.png \
        Husky_1

    ${PYTHON3} ./plot/plot_bar.py \
        ${OUTPUT_PATH}/bow_husky_2.csv \
        ${OUTPUT_PATH}/bow_husky_2.png \
        Husky_2

    ${PYTHON3} ./plot/plot_bar.py \
        ${OUTPUT_PATH}/bow_husky_3.csv \
        ${OUTPUT_PATH}/bow_husky_3.png \
        Husky_3

    ${PYTHON3} ./plot/plot_bar.py \
        ${OUTPUT_PATH}/bow_puppy.csv \
        ${OUTPUT_PATH}/bow_puppy.png \
        Puppy_1

     ${PYTHON3} ./plot/plot_group_bar.py \
        ${OUTPUT_PATH}/match_bow_husky_1.csv \
        ${OUTPUT_PATH}/match_bow_husky_1.png \
        Husky_1 Husky_3

    ${PYTHON3} ./plot/plot_group_bar.py \
        ${OUTPUT_PATH}/match_bow_husky_2.csv \
        ${OUTPUT_PATH}/match_bow_husky_2.png \
        Husky_2 Husky_3

    ${PYTHON3} ./plot/plot_group_bar.py \
        ${OUTPUT_PATH}/match_bow_puppy.csv \
        ${OUTPUT_PATH}/match_bow_puppy.png \
        Puppy_1 Husky_3
    echo "Plot Done"
fi

echo "Done"
exit ${SUCCESS}
