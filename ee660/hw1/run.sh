#!/bin/bash

mkdir -p ./out

python3 ./main.py \
    --x-train ./data/x_train.csv --y-train ./data/y_train.csv \
    --x-test ./data/x_test.csv --y-test ./data/y_test.csv \
    -d 1 2 3 7 10 \
    --scatter ./out/scatter.png \
    --mse-train ./out/mse_train.png \
    --mse-test ./out/mse_test.png

python3 ./main.py \
    --x-train ./data/x_train.csv --y-train ./data/y_train.csv \
    --x-test ./data/x_test.csv --y-test ./data/y_test.csv \
    -d 7 \
    -l 1e-5 1e-3 1e-1 1 1e1 \
    --mse-lambda ./out/mse_lambda.png
