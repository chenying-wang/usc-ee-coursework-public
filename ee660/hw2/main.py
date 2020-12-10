#!/usr/bin/python3

import sklearn.linear_model
import sklearn.model_selection
import sklearn.preprocessing
import numpy as np
import matplotlib.pyplot as plt
import argparse

COLOR = ['#1453ad', '#ad1453']

def train_test(x_train: np.ndarray, y_train: np.ndarray,
        x_test: np.ndarray, y_test: np.ndarray,
        reg_lambda: float = 1.0, std: bool = False) -> tuple:
    x_train_std, x_test_std = x_train, x_test
    if std:
        scaler = sklearn.preprocessing.StandardScaler()
        x_train_std = scaler.fit_transform(x_train)
        x_test_std = scaler.transform(x_test)
    clf = sklearn.linear_model.LogisticRegression(C = 1 / reg_lambda, max_iter = 1000).fit(x_train_std, y_train)
    error_rate_train = 1 - clf.score(x_train_std, y_train)
    error_rate_test =  1 - clf.score(x_test_std, y_test)
    return error_rate_train, error_rate_test

def cross_validation(x: np.ndarray, y: np.ndarray, reg_lambda: float = 1.0, std: bool = False) -> float:
    skf = sklearn.model_selection.StratifiedKFold(n_splits = 5, shuffle = True)
    error_rates = []
    for train_index, test_index in skf.split(x, y):
        _, error_rate = train_test(x[train_index], y[train_index], x[test_index], y[test_index],
            reg_lambda = reg_lambda, std = std)
        error_rates.append(error_rate)
    return np.mean(error_rates)

def part_a(x_train: np.ndarray, y_train: np.ndarray,
        x_test: np.ndarray, y_test: np.ndarray) -> None:
    print('STD')
    reg_lambdas = [1e-4, 1e-3, 1e-2, 5e-2, 1e-1, 5e-1, 1, 1e1]
    error_rates = []
    for l in reg_lambdas:
        er = cross_validation(x_train, y_train, reg_lambda = l, std = True)
        error_rates.append(er)
        print('%.5f\t%.6f' %  (l, er))

    opt_lambda = 1e-2
    print('Selected Lambda: %.5f' % opt_lambda)

    scaler = sklearn.preprocessing.StandardScaler()
    x_train_std = scaler.fit_transform(x_train)
    x_test_std = scaler.transform(x_test)
    error_rate_train, error_rate_test = train_test(x_train_std, y_train, x_test_std, y_test, reg_lambda = opt_lambda)
    print('ER Tr: %.6f, ER Test %.6f' %  (error_rate_train, error_rate_test))

    print('\nLOG')
    x_train_log = np.log(x_train + 0.1)
    x_test_log = np.log(x_test + 0.1)
    error_rate_cv = cross_validation(x_train_log, y_train, reg_lambda = opt_lambda)
    error_rate_train, error_rate_test = train_test(x_train_log, y_train, x_test_log, y_test, reg_lambda = opt_lambda)
    print('ER CV: %.6f, ER Tr: %.6f, ER Test %.6f' %  (error_rate_cv, error_rate_train, error_rate_test))

    print('\nBINARY')
    x_train_binary = np.where(x_train > 0, 1, 0)
    x_test_binary = np.where(x_test > 0, 1, 0)
    error_rate_cv = cross_validation(x_train_binary, y_train, reg_lambda = opt_lambda)
    error_rate_train, error_rate_test = train_test(x_train_binary, y_train, x_test_binary, y_test, reg_lambda = opt_lambda)
    print('ER CV: %.6f, ER Tr: %.6f, ER Test %.6f' %  (error_rate_cv, error_rate_train, error_rate_test))

def hist(x: np.ndarray, y: np.ndarray) -> tuple:
    bins = (np.unique(x), np.unique(y))
    hist, x_edges, y_edges = np.histogram2d(x, y, bins = bins)
    hist_x, hist_y = np.meshgrid(x_edges[:-1], y_edges[:-1])
    return hist_x.ravel(), hist_y.ravel(), hist.T.ravel()

def part_b(x: np.ndarray, y: np.ndarray,
        scatter_fname: str, non_spam_hist_fname: str, spam_hist_fname: str) -> None:
    x_binary = np.where(x > 0, 1, 0)
    x_scatter = np.sum(x_binary[:, 0:48], axis = 1)
    y_scatter = np.sum(x_binary[:, 48:54], axis = 1)

    x_scatter_non_spam = x_scatter[y == 0]
    y_scatter_non_spam = y_scatter[y == 0]
    x_scatter_spam = x_scatter[y == 1]
    y_scatter_spam = y_scatter[y == 1]

    fig, ax = plt.subplots()
    ax.scatter(x_scatter_non_spam, y_scatter_non_spam, label = 'Non-Spam',
        marker = 'o', c = COLOR[0], s = 12, alpha = 0.3)
    ax.scatter(x_scatter_spam, y_scatter_spam, label = 'Spam',
        marker = 'x', c = COLOR[1], s = 24, alpha = 0.3)
    ax.grid()
    ax.legend()
    fig.savefig(scatter_fname, dpi = 600, transparent = True)
    plt.clf()

    hist_x, hist_y, hist_z =  hist(x_scatter_non_spam, y_scatter_non_spam)
    dx = 0.75
    dy = 0.2

    fig = plt.figure()
    ax = fig.add_subplot(projection = '3d')
    ax.bar3d(hist_x, hist_y, 0, dx, dy, hist_z, label = 'Non-Spam', color = COLOR[0])
    ax.grid()
    fig.savefig(non_spam_hist_fname, dpi = 600, transparent = True)
    plt.clf()

    hist_x, hist_y, hist_z =  hist(x_scatter_spam, y_scatter_spam)
    dx = 0.75
    dy = 0.2

    fig = plt.figure()
    ax = fig.add_subplot(projection = '3d')
    ax.bar3d(hist_x, hist_y, 0, dx, dy, hist_z, label = 'Spam', color = COLOR[1])
    ax.grid()
    fig.savefig(spam_hist_fname, dpi = 600, transparent = True)
    plt.clf()

def main(x_train: np.ndarray, y_train: np.ndarray,
        x_test: np.ndarray, y_test: np.ndarray,
        scatter_fname: str, non_spam_hist_fname: str, spam_hist_fname: str) -> None:
    part_a(x_train, y_train, x_test, y_test)
    part_b(x_test, y_test, scatter_fname, non_spam_hist_fname, spam_hist_fname)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--x-train', help = 'x_train file', required = True)
    parser.add_argument('--y-train', help = 'y_train file', required = True)
    parser.add_argument('--x-test', help = 'x_test file', required = True)
    parser.add_argument('--y-test', help = 'y_test file', required = True)
    parser.add_argument('--scatter', help = 'scatter plot file', default = './scatter.png')
    parser.add_argument('--non-spam-hist', help = 'non spam hist plot file', default = './non_spam_hist.png')
    parser.add_argument('--spam-hist', help = 'spam hist plot file', default = './spam_hist.png')
    args = parser.parse_args()

    x_train, y_train = np.genfromtxt(args.x_train, delimiter = ','), np.genfromtxt(args.y_train, delimiter = ',')
    x_test, y_test = np.genfromtxt(args.x_test, delimiter = ','), np.genfromtxt(args.y_test, delimiter = ',')
    main(x_train, y_train, x_test, y_test, scatter_fname = args.scatter,
        non_spam_hist_fname = args.non_spam_hist, spam_hist_fname = args.spam_hist)
