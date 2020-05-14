# !/bin/bash

python3 ./prob1.py \
    --feature ./data/HW8_1_csv/train_x.csv \
    --label ./data/HW8_1_csv/train_y.csv \
    -c 1.0 \
    --graph ./graph1.png

python3 ./prob1.py \
    --feature ./data/HW8_1_csv/train_x.csv \
    --label ./data/HW8_1_csv/train_y.csv \
    -c 100.0 \
    --graph ./graph2.png

python3 ./prob1.py \
    --feature ./data/HW8_1_csv/train_x.csv \
    --label ./data/HW8_1_csv/train_y.csv \
    -c 100.0 \
    --graph ./graph3.png \
    --graph-sv

python3 ./prob1.py \
    --feature ./data/HW8_1_csv/train_x.csv \
    --label ./data/HW8_1_csv/train_y.csv \
    -c 1000.0

python3 ./prob1.py \
    --feature ./data/HW8_2_csv/train_x.csv \
    --label ./data/HW8_2_csv/train_y.csv \
    -c 50.0 \
    --kernel rbf \
    --graph ./graph4.png

python3 ./prob1.py \
    --feature ./data/HW8_2_csv/train_x.csv \
    --label ./data/HW8_2_csv/train_y.csv \
    -c 5000.0 \
    --kernel rbf \
    --graph ./graph5.png

python3 ./prob1.py \
    --feature ./data/HW8_2_csv/train_x.csv \
    --label ./data/HW8_2_csv/train_y.csv \
    --kernel rbf \
    --gamma 10.0 \
    --graph ./graph6.png

python3 ./prob1.py \
    --feature ./data/HW8_2_csv/train_x.csv \
    --label ./data/HW8_2_csv/train_y.csv \
    --kernel rbf \
    --gamma 50.0 \
    --graph ./graph7.png

python3 ./prob1.py \
    --feature ./data/HW8_2_csv/train_x.csv \
    --label ./data/HW8_2_csv/train_y.csv \
    --kernel rbf \
    --gamma 500.0 \
    --graph ./graph8.png

python3 ./prob2.py \
    --feature ./data/wine_csv/feature_train.csv \
    --label ./data/wine_csv/label_train.csv \
    -c 1.0 \
    --gamma 1.0

python3 ./prob2.py \
    --feature ./data/wine_csv/feature_train.csv \
    --label ./data/wine_csv/label_train.csv \
    --acc-graph ./graph9.png \
    --dev-graph ./graph10.png

python3 ./prob2.py \
    --feature ./data/wine_csv/feature_train.csv \
    --label ./data/wine_csv/label_train.csv \
    -c 1.5848931924611136 \
    --gamma 0.8912509381337456 \
    --times 20

python3 ./prob2.py \
    --feature ./data/wine_csv/feature_train.csv \
    --label ./data/wine_csv/label_train.csv \
    --acc-graph ./graph11.png \
    --dev-graph ./graph12.png \
    --times 20

python3 ./prob2.py \
    --feature ./data/wine_csv/feature_train.csv \
    --label ./data/wine_csv/label_train.csv \
    -c 0.28183829312644537 \
    --gamma 2.2387211385683394

python3 ./prob2.py \
    --feature ./data/wine_csv/feature_train.csv \
    --label ./data/wine_csv/label_train.csv \
    --feature-test ./data/wine_csv/feature_test.csv \
    --label-test ./data/wine_csv/label_test.csv \
    -c 0.28183829312644537 \
    --gamma 2.2387211385683394
