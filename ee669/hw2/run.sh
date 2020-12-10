#!/bin/bash

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export LD_LIBRARY_PATH="${PWD}/build/lib:${LD_LIBRARY_PATH}"

BIN="./bin/encoder"
CONTEXT_BITS=("0" "1" "2" "4")
PREPROCESSING_METHODS=("zigzag" "zorder" "hilbert")

for ctx_bits in ${CONTEXT_BITS[*]}; do
    ${BIN} --context-bits ${ctx_bits} -o ./out/image_${ctx_bits}.bin -i ./data/image.dat
    ${BIN} --context-bits ${ctx_bits} -o ./out/text_${ctx_bits}.bin -i ./data/text.dat
    ${BIN} --context-bits ${ctx_bits} -o ./out/audio_${ctx_bits}.bin -i ./data/audio.dat
    ${BIN} --context-bits ${ctx_bits} -o ./out/binary_${ctx_bits}.bin -i ./data/binary.dat

    ${BIN} --bit-plane --context-bits ${ctx_bits} -o ./out/image_bit_plane_${ctx_bits}.bin -i ./data/image.dat
    ${BIN} --bit-plane --context-bits ${ctx_bits} -o ./out/text_bit_plane_${ctx_bits}.bin -i ./data/text.dat
    ${BIN} --bit-plane --context-bits ${ctx_bits} -o ./out/audio_bit_plane_${ctx_bits}.bin -i ./data/audio.dat
    ${BIN} --bit-plane --context-bits ${ctx_bits} -o ./out/binary_bit_plane_${ctx_bits}.bin -i ./data/binary.dat

    for preproc in ${PREPROCESSING_METHODS[*]}; do
        ${BIN} --preprocessing ${preproc} --context-bits ${ctx_bits} -o ./out/image_${preproc}_${ctx_bits}.bin --width 256 --height 256 -i ./data/image.dat
        ${BIN} --preprocessing ${preproc} --bit-plane --context-bits ${ctx_bits} -o ./out/image_${preproc}_bit_plane_${ctx_bits}.bin --width 256 --height 256 -i ./data/image.dat
    done
done
