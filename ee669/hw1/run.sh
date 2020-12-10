#!/bin/bash

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export LD_LIBRARY_PATH="${PWD}/build/lib:${LD_LIBRARY_PATH}"

BIN="./bin/encoder"
ALGORITHMS=("SF" "Huffman" "AdaHuffman")

for algo in ${ALGORITHMS[*]}; do
    ${BIN} -a ${algo} -o ./out/audio_${algo}.bin -i ./data/audio.dat
    ${BIN} -a ${algo} -o ./out/text_${algo}.bin -i ./data/text.dat
    ${BIN} -a ${algo} -o ./out/binary_${algo}.bin -i ./data/binary.dat
    ${BIN} -a ${algo} -o ./out/image_${algo}.bin -i ./data/image.dat
done
