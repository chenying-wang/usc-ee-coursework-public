#!/usr/bin/python3

import sklearn
import sklearn.svm
import numpy as np
import matplotlib.pyplot as plt
import argparse

from plotSVMBoundaries import plotSVMBoundaries
from utils import load_csv, accuracy

def main(x, y, graph, C, kernel, gamma, graph_sv = False):
    if (np.shape(x)[0] != np.shape(y)[0]):
        raise RuntimeError('Size of feature and label data do not match')

    print('C = %.2f, Kernel = %s, Gamma = %.2f' % (C, kernel, gamma))
    if gamma <= 0:
        gamma = 'auto'
    clf = sklearn.svm.SVC(C = C, kernel = kernel, gamma = gamma)
    clf.fit(x, y)
    print('Accuracy: %.2f' % accuracy(clf.predict(x), y))
    if kernel == 'linear':
        print('Decision Function: ', clf.intercept_, clf.coef_)
        print('Support Vectors: ', clf.support_vectors_)
        print('Support at Margin? ', clf.decision_function(clf.support_vectors_))

    if graph != None:
        if graph_sv:
            plt = plotSVMBoundaries(x, y, clf, clf.support_vectors_)
        else :
            plt = plotSVMBoundaries(x, y, clf, [])
        plt.savefig(graph)
        plt.clf()

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser()
    parser.add_argument('--feature')
    parser.add_argument('--label')
    parser.add_argument('--graph')
    parser.add_argument('-c', type = float, default = 1.0)
    parser.add_argument('--kernel', default = 'linear')
    parser.add_argument('--gamma', type = float, default = 0.0)
    parser.add_argument('--graph-sv', action='store_true')
    args = parser.parse_args()

    try:
        x, y = load_csv(args.feature, args.label)
        main(x, y, args.graph, args.c, args.kernel, args.gamma, args.graph_sv)
    except RuntimeError as e:
        print('RuntimeError raised\n', e.args)
        exit(1)
