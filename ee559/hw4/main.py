#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import argparse

import classifier as c

def load_csv(training_csv, test_csv):
    print('Training Data: %s' % training_csv)
    print('Test Data: %s' % test_csv)

    training_data = np.loadtxt(training_csv, delimiter = ',')
    test_data = np.loadtxt(test_csv, delimiter = ',')
    return (training_data, test_data)

def load_separate_csv(training_feature_csv, training_label_csv, \
        test_feature_csv, test_label_csv):
    print('Training Feature: %s' % training_feature_csv)
    print('Training Label: %s' % training_label_csv)
    print('Test Feature: %s' % test_feature_csv)
    print('Test Label: %s' % test_label_csv)

    training_feature_data = np.loadtxt(training_feature_csv, delimiter = ',')
    training_label_data = np.loadtxt(training_label_csv, delimiter = ',')
    test_feature_data = np.loadtxt(test_feature_csv, delimiter = ',')
    test_label_data = np.loadtxt(test_label_csv, delimiter = ',')

    if (np.shape(training_feature_data)[0] != np.shape(training_label_data)[0]):
        raise RuntimeError('Size of training feature and label data do not match')
        return
    if (np.shape(test_feature_data)[0] != np.shape(test_label_data)[0]):
        raise RuntimeError('Size of test feature and label data do not match')
        return

    training_data = np.concatenate( \
        (training_feature_data, np.transpose([training_label_data])), axis = 1)
    test_data = np.concatenate( \
        (test_feature_data, np.transpose([test_label_data])), axis = 1)
    return (training_data, test_data)

def main(training_data, test_data, epoch, plot_fname):
    if (np.shape(training_data)[1] != np.shape(test_data)[1]):
        raise RuntimeError('Size of training and test data do not match')
        return

    classifier = c.GradientDecentClassifier()
    classifier = classifier.train(training_data, epoch, learning_rate = 1)
    print('Weight: %s' % classifier.weights_group[classifier.classifications == 1])
    print('Error Rate on Training Dataset: %.4f' % classifier.test(training_data))
    print('Error Rate on Test Dataset: %.4f' % classifier.test(test_data))

    fig, ax = plt.subplots()
    classifier.plot_decision_regions_boundaries(ax)
    classifier.plot_features(ax)
    classifier.plot_setlim(ax)
    classifier.plot_legend(ax)
    fig.savefig(plot_fname)
    plt.clf()

    return classifier

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser()
    parser.add_argument('--train', help = 'training csv')
    parser.add_argument('--test', help = 'test csv')
    parser.add_argument('--epoch', help = 'training epoch', type = int)
    parser.add_argument('--plot', help = 'plot file name')
    parser.add_argument('--train-feature', help = 'training feature csv')
    parser.add_argument('--train-label', help = 'training label csv')
    parser.add_argument('--test-feature', help = 'test feature csv')
    parser.add_argument('--test-label', help = 'test label csv')
    args = parser.parse_args()

    if (args.train and args.test):
        (training_data, test_data) = load_csv(args.train, args.test)
    else:
        (training_data, test_data) = load_separate_csv( \
            args.train_feature, args.train_label, \
            args.test_feature, args.test_label)

    main(training_data, test_data, args.epoch, args.plot)
