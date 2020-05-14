#!/bin/bash

python3 ./prob1.py \
    --train ./data/wine_train.csv \
    --test ./data/wine_test.csv

python3 ./prob2.py \
    --graph1 ./graph1.png \
    --graph2 ./graph2.png \
    --graph3 ./graph3.png
