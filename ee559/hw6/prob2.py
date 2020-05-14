#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import argparse

from plot_area import plot_area
from matplotlib.patches import Ellipse

COLOR = ['tab:blue', 'tab:orange']

def plot_feature(ax, feature, label):
    for i in range(np.shape(feature)[0]):
        ax.scatter(feature[i, 0], feature[i, 1], \
                    label = 'Class ' + str(label[i]), \
                    marker = 'x', alpha = 0.8, color = COLOR[label[i] - 1])

def plot_legend(ax, **kwargs):
        handles, labels = ax.get_legend_handles_labels()
        newLabels, newHandles = [], []
        for handle, label in zip(handles, labels):
            if label not in newLabels:
                newLabels.append(label)
                newHandles.append(handle)
        ax.legend(newHandles, newLabels, **kwargs)

def main(feature, label, plot1_name, plot2_name, plot3_name):
    if (np.shape(feature)[0] != np.size(label)):
        raise RuntimeError('Size of feature and label do not match')

    # (a)
    fig, ax = plt.subplots()
    plot_feature(ax, feature, label)
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    plot_legend(ax)
    fig.savefig(plot1_name)
    plt.clf()

    # (c)
    anglespace = np.arange(-2, 6, 0.002)
    fig, ax = plt.subplots()
    plot_feature(ax, feature ** 2, label)
    plot_area(ax, [[[5, -10, -2]]], anglespace, label = 'Gamma ' + str(1),
        color = COLOR[0], alpha = 0.6)
    plot_area(ax, [[[-5, 10, 2]]], anglespace, label = 'Gamma ' + str(2),
        color = COLOR[1], alpha = 0.6)
    ax.set_xlim(-1, 5)
    ax.set_ylim(-1, 5)
    plot_legend(ax)
    fig.savefig(plot2_name)
    plt.clf()

    # (d)
    fig, ax = plt.subplots()
    plot_feature(ax, feature, label)
    linespace = np.arange(-3, np.sqrt(0.5), 0.002)
    anglespace = np.linspace(0., 2 * np.pi, 100)
    ellipse = np.array([np.sqrt(0.5) * np.cos(anglespace), np.sqrt(2.5) * np.sin(anglespace)])
    plt.plot(ellipse[0], ellipse[1], color = 'black', alpha = 0.5)
    plt.fill(ellipse[0], ellipse[1], color = COLOR[0], alpha = 0.6, linewidth = 0)
    plt.fill_between(np.arange(-3, -np.sqrt(0.5), 0.002), -3, 3, color = COLOR[1], alpha = 0.6, linewidth = 0)
    plt.fill_between(ellipse[0], ellipse[1], 3, where = ellipse[1] > 0, color = COLOR[1], alpha = 0.6, linewidth = 0)
    plt.fill_between(ellipse[0], -3, ellipse[1], where = ellipse[1] < 0, color = COLOR[1], alpha = 0.6, linewidth = 0)
    plt.fill_between(np.arange(np.sqrt(0.5), 3, 0.002), -3, 3, color = COLOR[1], alpha = 0.6, linewidth = 0)
    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    plot_legend(ax)
    fig.savefig(plot3_name)
    plt.clf()

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser()
    parser.add_argument('--graph1')
    parser.add_argument('--graph2')
    parser.add_argument('--graph3')
    args = parser.parse_args()

    try:
        main(np.array([[0, 0], [0, 1], [0, -1],
                       [-2, 0], [-1, 0], [0, 2],
                       [0, -2], [1, 0], [2, 0]]),
            np.array([1, 1, 1, 2, 2, 2, 2, 2, 2]),
            args.graph1, args.graph2, args.graph3)
    except RuntimeError as e:
        print("RuntimeError raised\n", e.args)
        exit(1)
