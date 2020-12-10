#!/bin/bash

python3 ./main.py \
    --x-train ./data/Xtrain.csv --y-train ./data/ytrain.csv \
    --x-test ./data/Xtest.csv --y-test ./data/ytest.csv \
    --scatter ./scatter.png \
    --non-spam-hist ./non_spam_hist.png \
    --spam-hist ./spam_hist.png
