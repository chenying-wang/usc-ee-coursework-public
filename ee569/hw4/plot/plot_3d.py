#!/usr/bin/python3

# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, March 20, 2020

import numpy as np
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d
import sys

COLOR = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red']

def main(input_csv, output_file):
    features = np.loadtxt(input_csv, delimiter = ',', usecols = [0, 1, 2])
    label = np.loadtxt(input_csv, delimiter = ',', usecols = -1, dtype = 'str')
    labelSet = np.unique(label)
    fig = plt.figure()
    ax = fig.add_subplot(111, projection = '3d')
    for i in range(np.size(labelSet)):
        _features = features \
                [label == labelSet[i], :]
        ax.scatter(_features[:, 0], _features[:, 1], _features[:, 2], \
                    label = labelSet[i], \
                    marker = 'x', alpha = 0.8, color = COLOR[i])
    ax.legend()
    fig.savefig(output_file)

if (__name__ == '__main__'):
    if (len(sys.argv) < 3):
        exit()
    main(sys.argv[1], sys.argv[2])
