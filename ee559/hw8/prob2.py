#!/usr/bin/python3

import sklearn
import sklearn.model_selection
import sklearn.svm
import numpy as np
import matplotlib.pyplot as plt
from concurrent.futures import ThreadPoolExecutor
import argparse
import os

from utils import load_csv, accuracy

FOLD = 5

def train_kfold(clf, x, y, skf, times, acc_mat, dev_mat, i, j):
    for k in range(times):
        _i = 0
        acc_arr = []
        for train_index, test_index in skf.split(x, y):
            clf.fit(x[train_index], y[train_index])
            acc_arr.append(accuracy(clf.predict(x[test_index]), y[test_index]))
            _i += 1
        acc_mat[k][i][j], dev_mat[k][i][j] = np.mean(acc_arr), np.std(acc_arr)

def plot(x, y, arr, graph):
    fig, ax = plt.subplots()
    c = ax.pcolormesh(x, y, arr, cmap='RdBu', vmin = np.min(arr), vmax = np.max(arr))
    fig.colorbar(c, ax = ax)
    ax.set_xlabel('lg(C)')
    ax.set_ylabel('lg(gamma)')
    plt.savefig(graph)
    plt.clf()

def main(x, y, x_test = [], y_test = [], \
    acc_graph = None, dev_graph = None, \
    C = 0.0, kernel = 'rbf', gamma = 0.0, \
    K = 5, times = 1):
    if (np.shape(x)[0] != np.shape(y)[0]):
        raise RuntimeError('Size of feature and label data do not match')

    print('C = %.4f, Kernel = %s, Gamma = %.4f, K = %d, T = %d' % \
        (C, kernel, gamma, K, times))
    if gamma <= 0:
        gamma = 'auto'

    mean = np.mean(x[:, 0:2], axis = 0)
    dev = np.std(x[:, 0:2], axis = 0, ddof = 1)
    x_std = (x[:, 0:2] - mean) / dev

    if np.size(x_test) > 0 and np.size(y_test) > 0:
        x_test_std = (x_test[:, 0:2] - mean) / dev
        clf = sklearn.svm.SVC(C = C, kernel = kernel, gamma = gamma)
        clf.fit(x_std, y)
        acc = accuracy(clf.predict(x_test_std), y_test)
        print('Final Accuracy: %.8f' % acc)
        return acc

    skf = sklearn.model_selection.StratifiedKFold(n_splits = K, shuffle = True)
    if C > 0 and gamma > 0:
        clf = sklearn.svm.SVC(C = C, kernel = kernel, gamma = gamma)
        acc_mat = np.empty([times, 1, 1])
        dev_mat = np.empty([times, 1, 1])
        train_kfold(clf, x_std, y, skf, times, acc_mat, dev_mat, 0, 0)
        print('Accuracy and Standard Deviation:\n %s' % \
            np.vstack((acc_mat.flatten(), dev_mat.flatten())).T)
        print('Avg Accuracy and Standard Deviation: %.6f, %.6f' % \
            (np.mean(acc_mat), np.mean(dev_mat)))
        return 0

    executor = ThreadPoolExecutor(min(64, os.cpu_count() + 8))
    i_arr = np.arange(-3, 3, 0.05)
    j_arr = np.arange(-3, 3, 0.05)
    acc_mat = np.empty([times, np.size(i_arr), np.size(j_arr)])
    dev_mat = np.empty([times, np.size(i_arr), np.size(j_arr)])
    future_list = []
    _i = 0
    for i in i_arr:
        _j = 0
        for j in j_arr:
            C, gamma = 10 ** i, 10 ** j
            clf = sklearn.svm.SVC(C = C, kernel = kernel, gamma = gamma)
            future = executor.submit(train_kfold, clf, x_std, y, skf, times, acc_mat, dev_mat, _i, _j)
            future_list.append(future)
            _j += 1
        _i += 1

    size = np.size(future_list)
    _i = 0
    for future in future_list:
        future.result()
        _i += 1
        print('%d/%d [%.2f%%]' % (_i, size, 100 * _i / size), end = '\r')
    executor.shutdown()
    print()

    acc = np.mean(acc_mat, axis = 0)
    dev = np.mean(dev_mat, axis = 0)
    candidates = np.argsort(acc.flatten())[-size // 50:][::-1]
    i, j = np.unravel_index(candidates[np.argmin(dev.flatten()[candidates])], [np.size(i_arr), np.size(j_arr)])
    print('lg(C) = %.2f, lg(gamma) = %.2f, Acc = %.6f, Dev = %.6f' % (i_arr[i], j_arr[j], acc[i][j], dev[i][j]))

    if acc_graph != None:
        plot(i_arr, j_arr, acc, acc_graph)
    if dev_graph != None:
        plot(i_arr, j_arr, dev, dev_graph)

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser()
    parser.add_argument('--feature')
    parser.add_argument('--label')
    parser.add_argument('--feature-test')
    parser.add_argument('--label-test')
    parser.add_argument('--acc-graph')
    parser.add_argument('--dev-graph')
    parser.add_argument('-c', type = float, default = 0.0)
    parser.add_argument('--kernel', default = 'rbf')
    parser.add_argument('--gamma', type = float, default = 0.0)
    parser.add_argument('-k', type = int, default = 5)
    parser.add_argument('--times', type = int, default = 1)
    args = parser.parse_args()

    try:
        x, y = load_csv(args.feature, args.label)
        x_test, y_test = load_csv(args.feature_test, args.label_test)
        main(x, y, x_test, y_test, args.acc_graph, args.dev_graph, args.c, args.kernel, args.gamma, args.k, args.times)
        print()
    except RuntimeError as e:
        print('RuntimeError raised\n', e.args)
        exit(1)
