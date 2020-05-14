#!/bin/bash

python3 ./main.py \
    --train ./data/synthetic1_train.csv \
    --test ./data/synthetic1_test.csv \
    --epoch 1000 --plot ./graph1.png

python3 ./main.py \
    --train ./data/synthetic2_train.csv \
    --test ./data/synthetic2_test.csv \
    --epoch 1000 --plot ./graph2.png

python3 ./main.py \
    --train-feature ./data/feature_train.csv \
    --train-label ./data/label_train.csv \
    --test-feature ./data/feature_test.csv \
    --test-label ./data/label_test.csv \
    --epoch 1000 --plot ./graph3.png
