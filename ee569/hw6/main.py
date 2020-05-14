#!/usr/bin/python3

# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, April 24, 2020

import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

import tensorflow as tf
import numpy as np
import logging

import model

MAX_PIXEL_VALUE = 255

def get_dataset(shuffle = True):
    (x_train, y_train), (x_test, y_test) = tf.keras.datasets.cifar10.load_data()
    mean = np.mean(x_train, axis = (0, 1, 2))
    dev = np.std(x_train, axis = (0, 1, 2), ddof = 1)
    x_train, x_test = x_train.astype(np.float32), x_test.astype(np.float32)
    x_train, x_test = x_train / MAX_PIXEL_VALUE, x_test / MAX_PIXEL_VALUE
    y_train, y_test = y_train.flatten(), y_test.flatten()

    if not shuffle:
        return (x_train, y_train), (x_test, y_test)

    idx_train = np.random.permutation(np.shape(x_train)[0])
    idx_test = np.random.permutation(np.shape(x_test)[0])
    return (x_train[idx_train], y_train[idx_train]), (x_test[idx_test], y_test[idx_test])

def train(x, y):
    pixelHopModel = model.PixelHopPP()
    pixelHopModel.fit(x_train, y_train)
    return pixelHopModel

def test(pixelHopModel, x, y):
    pixelHopModel.evaluate(x, y)

if __name__ == '__main__':
    logger = logging.getLogger('main')
    logger.setLevel(logging.INFO)
    logger_ch = logging.StreamHandler()
    logger_ch.setFormatter(logging.Formatter('%(message)s'))
    logger_ch.setLevel(logging.INFO)
    logger.addHandler(logger_ch)
    logger.info("""
    ################################################################
    #    Name: Chenying Wang
    #    Email: chenying.wang@usc.edu
    #    USC ID: ****-****-**
    #    Date: Friday, April 24, 2020
    ################################################################
    """)
    tf.get_logger().setLevel('WARNING')
    (x_train, y_train), (x_test, y_test) = get_dataset()
    pixelHopModel = train(x_train, y_train)
    pixelHopModel.summary()
    test(pixelHopModel, x_test, y_test)
