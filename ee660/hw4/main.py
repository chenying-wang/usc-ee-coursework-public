#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import argparse
from typing import Callable, Tuple

COLOR = ['#4285f4', '#db4437', '#f4b400', '#0f9d58', '#303030']

def fit(x1: float, x2: float) -> Tuple[float, float]:
    return x1 + x2, -x1 * x2

def main(x: np.ndarray, x_min: float, x_max: float, target_fn: Callable[[str, str], int], plot_fname: str = None) -> None:
    tr_size = len(x)
    a_arr = np.empty(tr_size)
    b_arr = np.empty(tr_size)
    for i in range(tr_size):
        a_arr[i], b_arr[i] = fit(x[i, 0], x[i, 1])

    eout_fn = lambda x: np.mean(np.square(a_arr * x + b_arr - target_fn(x)))
    avg_fn = lambda x: np.mean(a_arr) * x + np.mean(b_arr)
    bias_fn = lambda x: (avg_fn(x) - target_fn(x)) ** 2
    var_fn = lambda x: np.mean(np.square(a_arr * x + b_arr - avg_fn(x)))

    x_size = 10000
    x_arr = np.arange(x_min, x_max, (x_max - x_min) / x_size, dtype = np.float)
    eout_arr = np.empty(x_size)
    bias_arr = np.empty(x_size)
    var_arr = np.empty(x_size)
    for i in range(x_size):
        eout_arr[i] = eout_fn(x_arr[i])
        bias_arr[i] = bias_fn(x_arr[i])
        var_arr[i] = var_fn(x_arr[i])
    eout_avg = np.mean(eout_arr)
    bias_avg = np.mean(bias_arr)
    var_avg = np.mean(var_arr)
    print('E_out: %f' % eout_avg)
    print('bias + var: %f + %f = %f' % (bias_avg, var_avg, bias_avg + var_avg))

    if plot_fname == None:
        return
    avg_arr = np.empty(x_size)
    target_arr = np.empty(x_size)
    for i in range(x_size):
        target_arr[i] = target_fn(x_arr[i])
        avg_arr[i] = avg_fn(x_arr[i])

    fig, ax = plt.subplots()
    ax.plot(x_arr, avg_arr, color = COLOR[0], label = 'g_bar(x)')
    ax.plot(x_arr, target_arr, color = COLOR[1], label = 'f(x)')
    ax.plot(x_arr, bias_arr, color = COLOR[2], label = 'bias(x)')
    ax.plot(x_arr, var_arr, color = COLOR[3], label = 'var(x)')
    ax.plot(x_arr, eout_arr, color = COLOR[4], label = 'E_out(x)')
    ax.grid()
    ax.legend()
    fig.savefig(plot_fname, dpi = 600, transparent = True)
    plt.clf()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--plot', help = 'plot file', default = './plot.png')
    args = parser.parse_args()

    x_min, x_max = -1, 1
    rng = np.random.default_rng()
    x = rng.random((50000, 2)) * (x_max - x_min) + x_min
    main(x, x_min, x_max, lambda x: x * x, plot_fname = args.plot)
