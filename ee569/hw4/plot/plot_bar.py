#!/usr/bin/python3

# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, March 20, 2020

import numpy as np
import matplotlib.pyplot as plt
import sys

COLOR = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red']

def main(input_csv, output_file, label):
    features = np.loadtxt(input_csv, delimiter = ',', usecols = 1)
    labels = np.loadtxt(input_csv, delimiter = ',', usecols = 0, dtype = 'str')

    width = 0.4
    x = np.arange(np.size(labels))
    fig, ax = plt.subplots()

    ax.bar(x, features / np.sum(features), width, label = label)

    ax.set_xlabel('Codeword')
    ax.set_ylabel('Frequency')
    ax.legend()
    fig.savefig(output_file)

if (__name__ == '__main__'):
    if (len(sys.argv) < 3):
        exit()
    main(sys.argv[1], sys.argv[2], sys.argv[3])
