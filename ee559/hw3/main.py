#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import sys

import classifier as c
from plot_area import plot_area

def main(training_data, plot_fname, is_reflect, is_in_weight_space):
    classifier = c.AugmentedSpaceClassifier()
    classifier = classifier.train(training_data)

    fig, ax = plt.subplots()
    if is_reflect:
        classifier.reflect_features()
    if is_in_weight_space:
        classifier.plot_features_in_weight_space(ax)
        classifier.plot_weight_vector_in_weight_space(ax)
        classifier.plot_setlim_in_weight_space(ax)
    else:
        classifier.plot_positive_decision_regions_boundaries(ax)
        classifier.plot_features(ax)
        classifier.plot_setlim(ax)
    classifier.plot_legend(ax)
    fig.savefig(plot_fname)
    plt.clf()

if (__name__ == '__main__'):
    if (len(sys.argv) < 2):
        plot_fname = 'result.png'
    else:
        plot_fname = sys.argv[1]

    if (len(sys.argv) > 2 and int(sys.argv[2]) > 0):
        is_to_reflect = True
    else:
        is_to_reflect = False

    if (len(sys.argv) > 3 and int(sys.argv[3]) > 0):
        is_in_weight_space = True
    else:
        is_in_weight_space = False

    main(np.array([ \
        np.array([1, -3, 1]), \
        np.array([1, -5, 1]), \
        np.array([1,  1, 2]), \
        np.array([1, -1, 2])]), plot_fname, is_to_reflect, is_in_weight_space)
