#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import sys

import classifier as c
from plot_area import plot_area

X_RANGE = [0, 6]
Y_RANGE = [0, 6]

def main(weights, plot_fname):
    x = np.arange(X_RANGE[0], X_RANGE[1], 0.002)

    classifier = c.NearestMeanClassifier(2)
    classifier.train_by_weights_group(np.array([ \
        [ weights[0],  weights[1]], \
        [-weights[0],  weights[2]], \
        [-weights[1], -weights[2]]]), \
        np.array([1, 2, 3]))

    features = np.array([[4, 1], [1, 5], [0, 0], [2.5, 3]])
    classifications = classifier.evaluate(features)
    print('Features and Classifications\n', np.insert(features, [2], np.transpose([classifications]), axis = 1))

    fig, ax = plt.subplots()
    classifier.plot_decision_regions_boundaries(ax)
    classifier.plot_indeterminate_regions(ax)
    classifier.plot_setlim(ax)
    classifier.plot_legend(ax)
    plt.savefig(plot_fname)
    plt.clf()

if (__name__ == '__main__'):
    if (len(sys.argv) < 2):
        plot_fname = 'result.png'
    else:
        plot_fname = sys.argv[1]

    main(np.array([
        np.array([ 5, -1, -1]),
        np.array([ 3, -1,  0]),
        np.array([-1, -1,  1])
    ]), sys.argv[1])
