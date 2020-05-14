#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import sys

import classifier as c
from plotDecBoundaries import plotDecBoundaries

def error_rate(classifications, true_classifications):
    if (np.shape(classifications) != np.shape(true_classifications)):
        raise RuntimeError("Size not equal")
    return np.count_nonzero(classifications - true_classifications) / np.size(classifications)

def main(training_csv, test_csv, plot_fname = "", features = np.array([0, 1])):
    print ("Training Data: %s" % training_csv)
    print ("Test Data: %s" % test_csv)
    print("Features", features)

    features = np.append(features, -1)
    training_data = np.loadtxt(training_csv, delimiter = ',', usecols = features)
    test_data = np.loadtxt(test_csv, delimiter = ',', usecols = features)
    training_data_shape = np.shape(training_data)
    test_data_shape = np.shape(test_data)

    if (training_data_shape[1] != test_data_shape[1]):
        raise RuntimeError("Size of training and test data do not match")
        return

    classifier = c.NearestMeanClassifier(training_data_shape[1] - 1)
    classifier = classifier.train(training_data)

    classifications = classifier.evaluate(training_data[:, :-1])
    error_rate_training = error_rate(classifications, training_data[:, -1])
    print("Error-rate of training data classifications = %.4f" % error_rate_training)


    classifications = classifier.evaluate(test_data[:, :-1])
    error_rate_test = error_rate(classifications, test_data[:, -1])
    print("Error-rate of test data classifications = %.4f" % error_rate_test)

    if (len(plot_fname)):
        plt = plotDecBoundaries(training_data[:, :-1], training_data[:, -1], classifier.feature_means)
        plt.savefig(plot_fname)
        plt.clf()
    return (error_rate_training, error_rate_test)

if (__name__ == '__main__'):
    if (len(sys.argv) < 3):
        print("Unspecified traning data and/or test data")
        exit(1)
    elif (len(sys.argv) == 3):
        feature_size = 2
    else:
        feature_size = int(sys.argv[3])

    if (feature_size < 2):
        print("feature size has to be 2 or above")
        exit(1)

    training_csv = sys.argv[1]
    test_csv = sys.argv[2]

    error_rate_training_list = np.array([])
    error_rate_test_list = np.array([])
    features_list = np.array([])
    if (len(sys.argv) > 4):
        plot_fname = sys.argv[4]
    try:
        if (len(sys.argv) < 7):
            for i in range(feature_size - 1):
                for j in range(i + 1, feature_size):
                    features = np.array([i, j])
                    if (len(sys.argv) < 5):
                        (er_training, er_test) = main(training_csv, test_csv, features = features)
                    else:
                        (er_training, er_test) = main(training_csv, test_csv, plot_fname, features)
                    error_rate_training_list = np.append(error_rate_training_list, er_training)
                    error_rate_test_list = np.append(error_rate_test_list, er_test)
                    if (np.size(features_list) == 0):
                        features_list = np.array([features])
                    else:
                        features_list = np.concatenate((features_list, np.array([features])))
            if (feature_size > 2):
                print("Standard deviation of error-rate on traning/test: %s" % np.std([error_rate_training_list, error_rate_test_list], axis = 1))
                print("Minimum error-rate on training %.4f, with featrues: %s" % (np.min(error_rate_training_list), features_list[np.argmin(error_rate_training_list)]))
                print("Maximum error-rate on training %.4f, with featrues: %s" % (np.max(error_rate_training_list), features_list[np.argmax(error_rate_training_list)]))
                print("Minimum error-rate on test %.4f, with featrues: %s" % (np.min(error_rate_test_list), features_list[np.argmin(error_rate_test_list)]))
                print("Maximum error-rate on test %.4f, with featrues: %s" % (np.max(error_rate_test_list), features_list[np.argmax(error_rate_test_list)]))
        else:
            main(training_csv, test_csv, plot_fname, features = np.array([int(sys.argv[5]), int(sys.argv[6])]))
    except RuntimeError as e:
        print("RuntimeError raised\n", e.args)
        exit(1)
