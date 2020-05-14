#!/usr/bin/python3

import numpy as np

import argparse
import logging
import os

import models
import utils

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

POSTURES_TRAINING_DATA = './data/postures/D_train.csv'
POSTURES_TEST_DATA = './data/postures/D_test.csv'

LOGGER = logging.getLogger('main')
LOGGER.setLevel(logging.INFO)
LOGGER_CH = logging.StreamHandler()
LOGGER_CH.setFormatter(logging.Formatter('[%(asctime)s] [%(levelname)s] [%(name)s] %(message)s'))
LOGGER_CH.setLevel(logging.INFO)
LOGGER.addHandler(LOGGER_CH)

def main(clf):
    x_train, y_train = utils.load_csv(POSTURES_TRAINING_DATA, feature_idx = range(2, 39), label_idx = 1)
    x_test, y_test = utils.load_csv(POSTURES_TEST_DATA, feature_idx = range(2, 39), label_idx = 1)
    model = models.PosturesEstimator(clf, aggregation = True, knn_n_neighbors = 9, \
        svm_c = 10 ** 0.6, svm_gamma = 10 ** -2.0, \
        nn_validation = True)

    assert len(x_train) == len(y_train) and len(x_test) == len(y_test), 'Data size not match'

    model.fit(x_train, y_train)
    acc_train = model.evaluate(x_train, y_train, cm = 'training.png')
    LOGGER.info('Training Accuracy: %.6f' % acc_train)

    acc_cv = model.cross_validate(x_train, y_train)
    if acc_cv is not None:
        LOGGER.info('Cross Validation Accuracy: %.6f ± %.6f' % (np.mean(acc_cv), np.std(acc_cv)))

    acc_test = model.evaluate(x_test, y_test, cm = 'test.png')
    LOGGER.info('Test Accuracy: %.6f' % acc_test)

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser()
    parser.add_argument('--clf', help = 'Classifier')
    args = parser.parse_args()

    try:
        main(args.clf)
    except RuntimeError as e:
        print('RuntimeError raised\n', e.args)
        exit(1)
    exit(0)
