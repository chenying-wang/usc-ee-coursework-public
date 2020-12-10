#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

COLOR = ['#4285f4']

def main(plot_1_fname, plot_2_fname):
    x = np.arange(1e-3, 1, 1e-3)
    y_1 = np.log(1 / x - 1)
    y_2 = np.exp(y_1)

    fig, ax = plt.subplots()
    ax.plot(x, y_1, color = COLOR[0])
    ax.grid(alpha = 0.3)
    fig.savefig(plot_1_fname, dpi = 600, transparent = True)
    plt.clf()

    fig, ax = plt.subplots()
    ax.plot(x, y_2, color = COLOR[0])
    ax.grid(alpha = 0.3)
    fig.savefig(plot_2_fname, dpi = 600, transparent = True)
    plt.clf()


if __name__ == "__main__":
    main('./fig_a.pdf', './fig_b.pdf')
