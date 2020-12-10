#!/bin/bash

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export LD_LIBRARY_PATH="${PWD}/build/lib:${LD_LIBRARY_PATH}"

SEQUENCE_PATH="${PWD}/data"
RESULT_PATH="${PWD}/out"
mkdir -p ${RESULT_PATH}

ENCODER_BIN="./bin/encoder"
ENCODER_SEQUENCE="mother-daughter_cif.yuv"
ENCODER_SEQUENCE_SIZE=(352 288)
ENCODER_ME=("dia" "dia" "hex" "hex" "umh" "umh" "umh" "umh" "esa" "esa")
ENCODER_CONFIG=("" "--no-emv" "" "--no-emv" "" "--no-emv" "--no-etra" "--no-emv --no-etra" "" "--no-emv")
ENCODER_SUFFIX=("opt" "no_emv" "opt" "no_emv" "opt" "no_emv" "no_etra" "no_both" "opt" "no_emv")

for (( i = 0; i < ${#ENCODER_ME[@]} ; i++)); do
    me=${ENCODER_ME[${i}]}
    cfg=${ENCODER_CONFIG[${i}]}
    suffix=${ENCODER_SUFFIX[${i}]}
    ${ENCODER_BIN} --width ${ENCODER_SEQUENCE_SIZE[0]} --height ${ENCODER_SEQUENCE_SIZE[1]} --me ${me} ${cfg} \
        -o ${RESULT_PATH}/mother-daughter_${me}_${suffix}.h264 -i ${SEQUENCE_PATH}/${ENCODER_SEQUENCE}
done
