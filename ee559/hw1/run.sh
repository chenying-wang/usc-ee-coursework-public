#!/bin/bash

echo "================ Part (a) and (b) ================"
python3 ./main.py ./data/synthetic1_train.csv ./data/synthetic1_test.csv 2 ./graph1.png
python3 ./main.py ./data/synthetic2_train.csv ./data/synthetic2_test.csv 2 ./graph2.png
echo "================================"

echo "================ Part (c) ================"
python3 ./main.py ./data/wine_train.csv ./data/wine_test.csv 2 ./graph3.png
echo "================================"

echo "================ Part (d) and (e) ================"
python3 ./main.py ./data/wine_train.csv ./data/wine_test.csv 13
python3 ./main.py ./data/wine_train.csv ./data/wine_test.csv 13 ./graph4.png 0 11
python3 ./main.py ./data/wine_train.csv ./data/wine_test.csv 13 ./graph5.png 1 4
echo "================================"
