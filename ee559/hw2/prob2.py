#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import sys

import classifier as c
from plot_area import plot_area

def main(training_csv, test_csv, features, plot_fname, ovr_plot_fname_format):
    print("Training Data: %s" % training_csv)
    print("Test Data: %s" % test_csv)
    print("Features: %s" % features)

    features = np.append(features, -1)
    training_data = np.loadtxt(training_csv, delimiter = ',', usecols = features)
    test_data = np.loadtxt(test_csv, delimiter = ',', usecols = features)
    training_data_shape = np.shape(training_data)
    test_data_shape = np.shape(test_data)

    if (training_data_shape[1] != test_data_shape[1]):
        raise RuntimeError("Size of training and test data do not match")
        return

    classifier = c.NearestMeanClassifier(training_data_shape[1] - 1)
    classifier = classifier.train(training_data, 'ovr')

    accuracy = classifier.test(training_data[:, :-1], training_data[:, -1])
    print("Accuracy on training dataset: %.6f" % accuracy)
    accuracy = classifier.test(test_data[:, :-1], test_data[:, -1])
    print("Accuracy on test dataset: %.6f" % accuracy)

    for classification in classifier.classifications:
        fig, ax = plt.subplots()
        weights = classifier.get_boundary_weight_by_ovr(classification)
        plot_area(ax, [[weights]], classifier.get_plot_linespace(), \
            label = 's_' + str(int(classification)), alpha = 0.6)
        plot_area(ax, [[-weights]], classifier.get_plot_linespace(), \
            label = 's_' + str(int(classification)) + '_bar', alpha = 0.6)
        classifier.plot_features(ax)
        classifier.plot_means(ax)
        classifier.plot_setlim(ax)
        fig.legend()
        plt.savefig(ovr_plot_fname_format.replace('{}', str(int(classification)), 1))
        plt.clf()

    fig, ax = plt.subplots()
    classifier.plot_decision_regions_boundaries(ax)
    classifier.plot_indeterminate_regions(ax)
    classifier.plot_features(ax)
    classifier.plot_means(ax)
    classifier.plot_setlim(ax)
    classifier.plot_legend(ax, loc = 2)
    plt.savefig(plot_fname)
    plt.clf()

if (__name__ == '__main__'):
    if (len(sys.argv) < 3):
        print('Unspecified traning data and/or test data')
        exit(1)

    training_csv = sys.argv[1]
    test_csv = sys.argv[2]
    if (len(sys.argv) < 4):
        plot_fname = 'result.png'
    else:
        plot_fname = sys.argv[3]
    if (len(sys.argv) < 5):
        ovr_plot_fname_format = 'result_{}.png'
    else:
        ovr_plot_fname_format = sys.argv[4]

    if (len(sys.argv) < 7):
        features = np.array([0, 1])
    else:
        features = np.array([int(sys.argv[5]), int(sys.argv[6])])


    main(training_csv, test_csv, features, plot_fname, ovr_plot_fname_format)
