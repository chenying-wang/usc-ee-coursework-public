#!/usr/bin/python3

# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, March 20, 2020

import numpy as np
import matplotlib.pyplot as plt
import sys

COLOR = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red']

def main(input_csv, output_file, label1, label2):
    features = np.loadtxt(input_csv, delimiter = ',', usecols = [1, 2])
    labels = np.loadtxt(input_csv, delimiter = ',', usecols = 0, dtype = 'str')

    width = 0.4
    x = np.arange(np.size(labels))
    fig, ax = plt.subplots()

    ax.bar(x - width/2, features[:, 0] / np.sum(features[:, 0]), width, label = label1)
    ax.bar(x + width/2, features[:, 1] / np.sum(features[:, 1]), width, label = label2)

    ax.set_xlabel('Codeword')
    ax.set_ylabel('Frequency')
    ax.legend()
    fig.savefig(output_file)

if (__name__ == '__main__'):
    if (len(sys.argv) < 3):
        exit()
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
