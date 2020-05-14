#!/usr/bin/python3

# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, January 24, 2020

import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import make_interp_spline, BSpline
import sys

COLOR = ['#ff0000', '#00ff00', '#0000ff']
COLOR_LEN = len(COLOR)

def main(input_file, output_file, legend):
    data = np.loadtxt(input_file, delimiter = ',')

    shape = np.shape(data)
    legendLen = len(legend)
    data = data[np.argsort(data[:, 0])]

    non_zero_rows = np.where(np.any(data[:, 1:], axis = 1) == True)
    x_min_idx = max(np.min(non_zero_rows) - 10, 0)
    x_max_idx = min(np.max(non_zero_rows) + 10, shape[0] - 1)


    spl = make_interp_spline(data[:, 0], data[:, 1:], k = 3)
    x = np.linspace(data[x_min_idx, 0], data[x_max_idx, 0], num = shape[0])
    data_smooth = spl(x)

    for i in range(0, shape[1] - 1):
        if i < COLOR_LEN:
            plt.plot(x, data_smooth[:, i], '-', label = legend[i] if i < legendLen else i, color = COLOR[i])
        else:
            plt.plot(x, data_smooth[:, i], '-', label = legend[i] if i < legendLen else i)

    plt.xticks(np.concatenate([[data[x_min_idx, 0], data[x_max_idx, 0]],
        np.arange(0, data[x_min_idx, 0], -32),
        np.arange(0, data[x_max_idx, 0], 32)]))
    plt.legend()
    plt.savefig(output_file)

if (__name__ == '__main__'):
    if (len(sys.argv) < 2):
        exit()
    main(sys.argv[1], sys.argv[2], legend = sys.argv[3:])
