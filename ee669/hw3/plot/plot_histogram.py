#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import argparse

COLOR = '#4285F4'

def main(csv_fname, fig_fname):
    data = np.genfromtxt(csv_fname, delimiter = ',', dtype = np.int)
    fig, ax = plt.subplots()
    x, y = data[:, 0], data[:, 1]
    min_diff = np.min(np.diff(np.sort(x)))
    width = max(min(min_diff - 3, 20), 1.0)
    ax.bar(x, y, width = width, color = COLOR)
    if min_diff >= 8:
        ax.set_xticks(x)
        ax.set_xticklabels(x, rotation = 45)
    else:
        xticks = np.arange(np.min(x), np.max(x), 16)
        if np.max(x) >= xticks[-1] + 8:
            xticks = np.append(xticks, np.max(x))
        else:
            xticks[-1] = np.max(x)
        ax.set_xticks(xticks)
        ax.set_xticklabels(xticks, rotation = 45)
    ax.grid()
    ax.set_ylabel('Frequency')
    fig.savefig(fig_fname)
    plt.clf()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', help = 'Input CSV file', required = True)
    parser.add_argument('-o', help = 'Output figure file', required = True)
    args = parser.parse_args()

    main(args.i, args.o)
