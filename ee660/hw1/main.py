#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import argparse

COLOR = {
    'train': '#1453ad',
    'test': '#ad1453'
}

def plot_scatter(x: np.ndarray, y: np.ndarray, scatter_fname: str) -> None:
    fig, ax = plt.subplots()
    ax.scatter(x, y, color = COLOR['train'])
    ax.grid()
    fig.savefig(scatter_fname, dpi = 800, transparent=True)
    plt.clf()

def expand(x: np.ndarray, degree: int) -> np.ndarray:
    degree = max(0, degree)
    size = len(x)
    x_expansion = np.empty([size, degree + 1], dtype = x.dtype)
    x_expansion[:, 0] = 1
    for i in range(degree):
        x_expansion[:, i + 1] = x_expansion[:, i] * x
    return x_expansion

def ridge_regression(x: np.ndarray, y: np.ndarray, lambda_param: float) -> np.ndarray:
    lambda_param = max(0.0, lambda_param)
    if lambda_param == 0.0:
        return np.dot(np.linalg.pinv(x), y)
    weight = lambda_param * np.eye(np.shape(x)[1], dtype = x.dtype)
    weight += np.dot(x.T, x)
    weight = np.linalg.inv(weight)
    weight = np.dot(weight, x.T)
    weight = np.dot(weight, y)
    return weight

def main(x_train: np.ndarray, y_train: np.ndarray,
        x_test: np.ndarray, y_test: np.ndarray,
        degree: int, lambda_param: float = 0.0, plot: bool = True) -> tuple:
    x_train_expansion = expand(x_train, degree)
    weight = ridge_regression(x_train_expansion, y_train, lambda_param)
    y_train_regression = np.dot(x_train_expansion, weight)
    mse_train = np.mean(np.square(y_train - y_train_regression))
    x_test_expansion = expand(x_test, degree)
    y_test_regression = np.dot(x_test_expansion, weight)
    mse_test = np.mean(np.square(y_test - y_test_regression))

    if not plot:
        return weight, mse_train, mse_test
    fig, ax = plt.subplots()
    ax.scatter(x_test, y_test, color = '#1453ad')
    linespace = np.linspace(np.min(x_train), np.max(x_train), 1000)
    ax.plot(linespace, np.dot(expand(linespace, degree), weight))
    ax.grid()
    fig.savefig('./out/curve_' + str(degree) + '_' + str(lambda_param) + '.png', transparent=True)
    plt.clf()
    return weight, mse_train, mse_test

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--x-train', help = 'x_train file', required = True)
    parser.add_argument('--y-train', help = 'y_train file', required = True)
    parser.add_argument('--x-test', help = 'x_test file', required = True)
    parser.add_argument('--y-test', help = 'y_test file', required = True)
    parser.add_argument('-d', '--degree', nargs = '+', help = 'Degrees of polynomials', required = True)
    parser.add_argument('-l', '--lambda', nargs = '+', help = 'Lambda in idge regression')
    parser.add_argument('--scatter', help = 'Filename of scatter graph')
    parser.add_argument('--mse-train', help = 'Filename of MSE vs degree graph')
    parser.add_argument('--mse-test', help = 'Filename of MSE vs degree graph')
    parser.add_argument('--mse-lambda', help = 'Filename of MSE vs lambda graph')
    args = parser.parse_args()

    x_train, y_train = np.genfromtxt(args.x_train), np.genfromtxt(args.y_train)
    x_test, y_test = np.genfromtxt(args.x_test), np.genfromtxt(args.y_test)

    if args.scatter:
        plot_scatter(x_train, y_train, args.scatter)
    degrees = np.array(args.degree, dtype = np.int)
    if getattr(args, 'lambda') == None:
        setattr(args, 'lambda', ['0'])
    lambdas = np.array(getattr(args, 'lambda'), dtype = np.float64)

    if len(degrees) ==  1 or len(lambdas) == 1:
        mse_train_list = np.empty(len(degrees) * len(lambdas), dtype = np.float64)
        mse_test_list = np.empty(len(degrees) * len(lambdas), dtype = np.float64)
    for i in range(len(degrees)):
        for j in range(len(lambdas)):
            weight, mse_train, mse_test = main(x_train, y_train, x_test, y_test, degrees[i], lambdas[j])
            if len(lambdas) == 1:
                mse_train_list[i] = mse_train
                mse_test_list[i] = mse_test
            elif len(degrees) == 1:
                mse_train_list[j] = mse_train
                mse_test_list[j] = mse_test
            print('Degree: %d, Lambda: %f' % (degrees[i], lambdas[j]))
            print('Weight vector: %s' % weight)
            print('Training Data MSE: %f' % mse_train)
            print('Test Data MSE: %f' % mse_test)
            print('')

    if args.mse_train and len(lambdas) == 1:
        fig, ax = plt.subplots()
        ax.plot(degrees, mse_train_list, '-o', label = 'Training MSE', color = COLOR['train'])
        ax.set_xticks(degrees)
        ax.legend()
        ax.grid()
        fig.savefig(args.mse_train, dpi = 800, transparent=True)
        plt.clf()
    if args.mse_test and len(lambdas) == 1:
        fig, ax = plt.subplots()
        ax.plot(degrees, mse_test_list, '-o', label = 'Test MSE', color = COLOR['test'])
        ax.set_xticks(degrees)
        ax.legend()
        ax.grid()
        fig.savefig(args.mse_test, dpi = 800, transparent=True)
        plt.clf()
    if args.mse_lambda and len(degrees) == 1:
        fig, ax = plt.subplots()
        ax.plot(lambdas, mse_train_list, '-o', label = 'Training MSE', color = COLOR['train'])
        ax.plot(lambdas, mse_test_list, '-o', label = 'Test MSE', color = COLOR['test'])
        ax.set_xscale('log')
        ax.set_xticks(lambdas)
        ax.legend()
        ax.grid()
        fig.savefig(args.mse_lambda, dpi = 800, transparent=True)
        plt.clf()
