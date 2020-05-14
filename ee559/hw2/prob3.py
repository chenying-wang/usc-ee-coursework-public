#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import sys

import classifier as c
from plot_area import plot_area


def main(training_data, training_type, plot_fname):
    classifier = c.NearestMeanClassifier(2)
    classifier = classifier.train(training_data, training_type)

    fig, ax = plt.subplots()
    classifier.plot_decision_regions_boundaries(ax)
    classifier.plot_means(ax)
    classifier.plot_setlim(ax)
    classifier.plot_legend(ax)
    fig.savefig(plot_fname)
    plt.clf()

if (__name__ == '__main__'):
    if (len(sys.argv) < 2):
        plot_fname_b = 'result.png'
    else:
        plot_fname_b = sys.argv[1]

    if (len(sys.argv) < 3):
        plot_fname_d = 'result.png'
    else:
        plot_fname_d = sys.argv[2]

    main(np.array([ \
        np.array([0, -2, 1]), \
        np.array([0,  1, 2])]), 'ovo', plot_fname_b)
    main(np.array([ \
        np.array([0, -2, 1]), \
        np.array([0,  1, 2]), \
        np.array([2,  0, 3])]), 'ovo', plot_fname_d)
