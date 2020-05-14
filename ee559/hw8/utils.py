import numpy as np
import os

def load_csv(features_csv, labels_csv):
    if features_csv != None and os.path.isfile(features_csv):
        x = np.loadtxt(features_csv, delimiter = ',')
    else:
        x = []
    if labels_csv != None and os.path.isfile(labels_csv):
        y = np.loadtxt(labels_csv, delimiter = ',')
    else:
        y = []
    return x, y

def accuracy(prediction, labels):
    if (np.shape(prediction) != np.shape(labels)):
        raise RuntimeError('Size not equal')
    return 1 - np.count_nonzero(prediction - labels) / np.size(labels)
