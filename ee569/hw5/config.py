# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, April 3, 2020

TRAIN_SIZE = 50000
TEST_SIZE = 10000

WIDTH = HEIGHT = 32
CHANNEL = 3
LINE_SIZE = 1 + CHANNEL * WIDTH * HEIGHT
MEAN = [125.3, 123.0, 113.9]
STD = [63.0, 62.1, 66.7]

BATCH_SIZE = 128

SGD_LEARNING_RATE = 0.01
SGD_LEARNING_RATE_DECAY_STEPS = 20
SGD_LEARNING_RATE_DECAY_RATE = 0.5
SGD_MOMENTUM = 0.9

WEIGHT_DECAY = 1e-4
DROPOUT_RATE = 0.1