#!/usr/bin/python3

print("""
################################################################
#    Name: Chenying Wang
#    Email: chenying.wang@usc.edu
#    USC ID: ****-****-**
#    Date: Friday, April 3, 2020
################################################################
""")

import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

import tensorflow as tf
import numpy as np
import matplotlib.pyplot as plt
import argparse

from model import LeNet5Model
from model import ModifiedLeNet5Model
from utils import nhwc2nchw
import config

def get_dataset(test_dataset_only = False, data_augmentation = False):
    (x_train, y_train), (x_test, y_test) = tf.keras.datasets.cifar10.load_data()
    x_train, x_test = nhwc2nchw(x_train), nhwc2nchw(x_test)
    mean = np.mean(x_train, axis = (0, 2, 3))
    dev = np.std(x_train, axis = (0, 2, 3), ddof = 1)
    x_train, x_test = x_train.astype('float32'), x_test.astype('float32')
    for c in range(np.shape(x_train)[1]):
        x_train[:, c, :, :] = (x_train[:, c, :, :] - mean[c]) / dev[c]
        x_test[:, c, :, :] = (x_test[:, c, :, :] - mean[c]) / dev[c]
    y_train, y_test = y_train.flatten(), y_test.flatten()

    test_dataset = tf.data.Dataset \
        .from_tensor_slices((x_test, y_test)) \
        .batch(config.TEST_SIZE // 20) \
        .cache() \
        .prefetch(tf.data.experimental.AUTOTUNE)
    if test_dataset_only:
        return test_dataset

    img_data_gen = tf.keras.preprocessing.image.ImageDataGenerator( \
        rotation_range = 15, horizontal_flip = True, zoom_range = 0.2, \
        width_shift_range = 0.1, height_shift_range = 0.1, \
        data_format = 'channels_first')

    training_dataset = tf.data.Dataset \
        .from_tensor_slices((x_train, y_train)) \
        .cache()
    if data_augmentation:
        training_dataset = training_dataset.map(lambda x, y: (augment(x, img_data_gen), y), \
            num_parallel_calls = tf.data.experimental.AUTOTUNE)
    training_dataset = training_dataset.shuffle(config.TRAIN_SIZE // 10) \
        .batch(config.BATCH_SIZE) \
        .prefetch(tf.data.experimental.AUTOTUNE)

    return training_dataset, test_dataset

def augment(x, img_data_gen):
    x_aug = tf.numpy_function(img_data_gen.random_transform, [x], tf.float32)
    x_aug.set_shape([config.CHANNEL, config.HEIGHT, config.WIDTH])
    return x_aug

def train(model, epoch, data_augmentation, training_data_csv = None, weights_filepath = None):
    training_dataset, test_dataset = get_dataset(data_augmentation = data_augmentation)

    if model == 'lenet_5':
        model = LeNet5Model((config.CHANNEL, config.HEIGHT, config.WIDTH), name = 'lenet_5')
    else:
        model = ModifiedLeNet5Model((config.CHANNEL, config.HEIGHT, config.WIDTH), name = 'mod_lenet_5')

    callbacks = []
    if training_data_csv != None:
        callbacks.append(tf.keras.callbacks.CSVLogger(training_data_csv))
    if weights_filepath != None:
        callbacks.append(tf.keras.callbacks.ModelCheckpoint(
            weights_filepath, monitor='val_accuracy', save_best_only = True, save_weights_only = True))

    history = model.fit(training_dataset, epochs = epoch, \
                        callbacks = callbacks, validation_data = test_dataset)

def test(model, weights_filepath):
    test_dataset = get_dataset(test_dataset_only = True)
    if model == 'lenet_5':
        model = LeNet5Model((config.CHANNEL, config.HEIGHT, config.WIDTH), name = 'lenet_5')
    else:
        model = ModifiedLeNet5Model((config.CHANNEL, config.HEIGHT, config.WIDTH), name = 'mod_lenet_5')

    model.load_weights(weights_filepath)
    model.evaluate(test_dataset)

def plot(fname):
    title = np.loadtxt(fname, dtype = str, delimiter = ',', max_rows = 1)
    data = np.loadtxt(fname, delimiter = ',', skiprows = 1)
    print('Max val_accuracy: %.4f' % np.max(data[:, 3]))

    fig, axs = plt.subplots(2, 1)
    axs[0].plot(data[:, 0], data[:, 1], label = title[1])
    axs[0].plot(data[:, 0], data[:, 3], label = title[3])
    axs[0].grid(True)
    axs[0].legend()

    axs[1].plot(data[:, 0], data[:, 2], label = title[2])
    axs[1].plot(data[:, 0], data[:, 4], label = title[4])
    axs[1].grid(True)
    axs[1].legend()

    plt.savefig(fname + '.png')
    plt.clf()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--train', action = 'store_true', default = False, help = 'To train model')
    parser.add_argument('--test', action = 'store_true', default = False, help = 'To test model')
    parser.add_argument('--plot', action = 'store_true', default = False, help = 'To plot training history')
    parser.add_argument('--model', help = 'Specify model')
    parser.add_argument('--weights', help = 'Weights filepath')
    parser.add_argument('--epoch', type = int, help = 'Specify epoch')
    parser.add_argument('--data-augmentation', action = 'store_true', default = False, help = 'To use data augmentation')
    parser.add_argument('--training-history', help = 'Store training History CSV')
    args = parser.parse_args()

    tf.get_logger().setLevel('WARNING')

    if args.train:
        train(args.model, args.epoch, args.data_augmentation, args.training_history, args.weights)
    elif args.test:
        test(args.model, args.weights)
    elif args.plot:
        plot(args.training_history)
