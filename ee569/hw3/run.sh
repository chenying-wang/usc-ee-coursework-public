#!/bin/bash

echo "
################################################################
# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, February 28, 2020
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

#### Warp Run ####
if [ ${WARP} -ne 0 ]; then
    echo "Warp Start"
    ${WARP_BIN} -a warp \
        -i ${WARP_INPUT_HEDWIG} \
        -o ${OUTPUT_IMAGE_PATH}/hedwig_warp.raw \
        -w ${WARP_WIDTH} -h ${WARP_HEIGHT} \
        -c ${WARP_CHANNEL} \
        > ${OUTPUT_PATH}/warp_hedwig.log

    ${WARP_BIN} -a warp \
        -i ${WARP_INPUT_RACCOON} \
        -o ${OUTPUT_IMAGE_PATH}/raccoon_warp.raw \
        -w ${WARP_WIDTH} -h ${WARP_HEIGHT} \
        -c ${WARP_CHANNEL} \
        > ${OUTPUT_PATH}/warp_raccoon.log

    ${WARP_BIN} -a warp \
        -i ${WARP_INPUT_BB8} \
        -o ${OUTPUT_IMAGE_PATH}/bb8_warp.raw \
        -w ${WARP_WIDTH} -h ${WARP_HEIGHT} \
        -c ${WARP_CHANNEL} \
        > ${OUTPUT_PATH}/warp_bb8.log

    ${WARP_BIN} -a reverse \
        -i ${OUTPUT_IMAGE_PATH}/hedwig_warp.raw \
        -o ${OUTPUT_IMAGE_PATH}/hedwig_reverse.raw \
        -w ${WARP_WIDTH} -h ${WARP_HEIGHT} \
        -c ${WARP_CHANNEL} \
        > ${OUTPUT_PATH}/warp_reverse_hedwig.log

    ${WARP_BIN} -a reverse \
        -i ${OUTPUT_IMAGE_PATH}/raccoon_warp.raw \
        -o ${OUTPUT_IMAGE_PATH}/raccoon_reverse.raw \
        -w ${WARP_WIDTH} -h ${WARP_HEIGHT} \
        -c ${WARP_CHANNEL} \
        > ${OUTPUT_PATH}/warp_reverse_raccoon.log

    ${WARP_BIN} -a reverse \
        -i ${OUTPUT_IMAGE_PATH}/bb8_warp.raw \
        -o ${OUTPUT_IMAGE_PATH}/bb8_reverse.raw \
        -w ${WARP_WIDTH} -h ${WARP_HEIGHT} \
        -c ${WARP_CHANNEL} \
        > ${OUTPUT_PATH}/warp_reverse_bb8.log
    echo "Warp Done"
fi

#### Stitch Run ####
if [ ${STITCH} -ne 0 ]; then
    echo "Stitch Start"
    ${STITCH_BIN} -a stitch \
        --left ${STITCH_INPUT_LEFT} \
        --middle ${STITCH_INPUT_MIDDLE} \
        --right ${STITCH_INPUT_RIGHT} \
        -o ${OUTPUT_IMAGE_PATH}/stitch.raw \
        -w ${STITCH_WIDTH} -h ${STITCH_HEIGHT} \
        -c ${STITCH_CHANNEL} \
        --lm-matches ${OUTPUT_IMAGE_PATH}/stitch_lm_matches.raw \
        --mr-matches ${OUTPUT_IMAGE_PATH}/stitch_mr_matches.raw \
        --lm-best-matches ${OUTPUT_IMAGE_PATH}/stitch_lm_best_matches.raw \
        --mr-best-matches ${OUTPUT_IMAGE_PATH}/stitch_mr_best_matches.raw \
        > ${OUTPUT_PATH}/stitch.log
    echo "Stitch Done"
fi

#### Morph Run ####
if [ ${MORPH} -ne 0 ]; then
    echo "Morph Start"
    ${MORPH_BIN} -a shrink \
        -i ${MORPH_INPUT_FAN} \
        -o ${OUTPUT_IMAGE_PATH}/fan_shrink.raw \
        -w ${MORPH_WIDTH_FAN} -h ${MORPH_HEIGHT_FAN} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/fan_shrink_track.raw \
        > ${OUTPUT_PATH}/morph_shrink_fan.log

    ${MORPH_BIN} -a thin \
        -i ${MORPH_INPUT_FAN} \
        -o ${OUTPUT_IMAGE_PATH}/fan_thin.raw \
        -w ${MORPH_WIDTH_FAN} -h ${MORPH_HEIGHT_FAN} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/fan_thin_track.raw \
        > ${OUTPUT_PATH}/morph_thin_fan.log

    ${MORPH_BIN} -a skeletonize \
        -i ${MORPH_INPUT_FAN} \
        -o ${OUTPUT_IMAGE_PATH}/fan_skeletonize.raw \
        -w ${MORPH_WIDTH_FAN} -h ${MORPH_HEIGHT_FAN} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/fan_skeletonize_track.raw \
        > ${OUTPUT_PATH}/morph_skeletonize_fan.log

    ${MORPH_BIN} -a shrink \
        -i ${MORPH_INPUT_CUP} \
        -o ${OUTPUT_IMAGE_PATH}/cup_shrink.raw \
        -w ${MORPH_WIDTH_CUP} -h ${MORPH_HEIGHT_CUP} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/cup_shrink_track.raw \
        > ${OUTPUT_PATH}/morph_shrink_cup.log

    ${MORPH_BIN} -a thin \
        -i ${MORPH_INPUT_CUP} \
        -o ${OUTPUT_IMAGE_PATH}/cup_thin.raw \
        -w ${MORPH_WIDTH_CUP} -h ${MORPH_HEIGHT_CUP} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/cup_thin_track.raw \
        > ${OUTPUT_PATH}/morph_thin_cup.log

    ${MORPH_BIN} -a skeletonize \
        -i ${MORPH_INPUT_CUP} \
        -o ${OUTPUT_IMAGE_PATH}/cup_skeletonize.raw \
        -w ${MORPH_WIDTH_CUP} -h ${MORPH_HEIGHT_CUP} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/cup_skeletonize_track.raw \
        > ${OUTPUT_PATH}/morph_skeletonize_cup.log

    ${MORPH_BIN} -a shrink \
        -i ${MORPH_INPUT_MAZE} \
        -o ${OUTPUT_IMAGE_PATH}/maze_shrink.raw \
        -w ${MORPH_WIDTH_MAZE} -h ${MORPH_HEIGHT_MAZE} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/maze_shrink_track.raw \
        > ${OUTPUT_PATH}/morph_shrink_maze.log

    ${MORPH_BIN} -a thin \
        -i ${MORPH_INPUT_MAZE} \
        -o ${OUTPUT_IMAGE_PATH}/maze_thin.raw \
        -w ${MORPH_WIDTH_MAZE} -h ${MORPH_HEIGHT_MAZE} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/maze_thin_track.raw \
        > ${OUTPUT_PATH}/morph_thin_maze.log

    ${MORPH_BIN} -a skeletonize \
        -i ${MORPH_INPUT_MAZE} \
        -o ${OUTPUT_IMAGE_PATH}/maze_skeletonize.raw \
        -w ${MORPH_WIDTH_MAZE} -h ${MORPH_HEIGHT_MAZE} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/maze_skeletonize_track.raw \
        > ${OUTPUT_PATH}/morph_skeletonize_maze.log

    ${MORPH_BIN} -a count \
        -i ${MORPH_INPUT_STARS} \
        -o ${OUTPUT_IMAGE_PATH}/stars_count.raw \
        -w ${MORPH_WIDTH_STARS} -h ${MORPH_HEIGHT_STARS} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --count-csv ${OUTPUT_PATH}/morph_count_stars.csv \
        > ${OUTPUT_PATH}/morph_count_stars.log

    ${MORPH_BIN} -a count_dfs \
        -i ${MORPH_INPUT_STARS} \
        -o ${OUTPUT_IMAGE_PATH}/stars_count_dfs.raw \
        -w ${MORPH_WIDTH_STARS} -h ${MORPH_HEIGHT_STARS} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --count-csv ${OUTPUT_PATH}/morph_count_dfs_stars.csv \
        > ${OUTPUT_PATH}/morph_count_dfs_stars.log

    ${MORPH_BIN} -a pcb_hole \
        -i ${MORPH_INPUT_PCB} \
        -o ${OUTPUT_IMAGE_PATH}/pcb_hole.raw \
        -w ${MORPH_WIDTH_PCB} -h ${MORPH_HEIGHT_PCB} \
        -c ${MORPH_CHANNEL} \
        --threshold ${MORPH_THRESHOLD_PCB} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        > ${OUTPUT_PATH}/morph_pcb_hole.log

    ${MORPH_BIN} -a pcb_pathway \
        -i ${MORPH_INPUT_PCB} \
        -o ${OUTPUT_IMAGE_PATH}/pcb_pathway.raw \
        -w ${MORPH_WIDTH_PCB} -h ${MORPH_HEIGHT_PCB} \
        -c ${MORPH_CHANNEL} \
        --threshold ${MORPH_THRESHOLD_PCB} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/pcb_pathway_track.raw \
        > ${OUTPUT_PATH}/morph_pcb_pathway.log

    ${MORPH_BIN} -a defect \
        -i ${MORPH_INPUT_GEAR} \
        -o ${OUTPUT_IMAGE_PATH}/geartooth_defect.raw \
        -w ${MORPH_WIDTH_GEAR} -h ${MORPH_HEIGHT_GEAR} \
        -c ${MORPH_CHANNEL} \
        --stage1-masks ${MORPH_STAGE1_MASKS} \
        --stage2-masks ${MORPH_STAGE2_MASKS} \
        --track ${OUTPUT_IMAGE_PATH}/geartooth_defect_track.raw \
        > ${OUTPUT_PATH}/morph_geartooth_defect.log
    echo "Morph Done"
fi

#### Plot Run ####
if [ ${PLOT} -ne 0 ]; then
    echo "Plot Start"
    if [ -f ${OUTPUT_PATH}/morph_count_stars.csv ]; then
        ${PYTHON3} ${PWD}/plot/simple_plot.py \
            ${OUTPUT_PATH}/morph_count_stars.csv \
            ${OUTPUT_PATH}/morph_count_stars.png \
            size
    fi
    if [ -f ${OUTPUT_PATH}/morph_count_dfs_stars.csv ]; then
        ${PYTHON3} ${PWD}/plot/simple_plot.py \
            ${OUTPUT_PATH}/morph_count_dfs_stars.csv \
            ${OUTPUT_PATH}/morph_count_dfs_stars.png \
            size
    fi
    echo "Plot Done"
fi

echo "Done"
exit ${SUCCESS}
