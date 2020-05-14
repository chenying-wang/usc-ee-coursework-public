import sklearn.metrics
import numpy as np

import os

def load_csv(csv, feature_idx, label_idx):
    if csv != None and os.path.isfile(csv):
        data = np.genfromtxt(csv, delimiter = ',', skip_header = 1, filling_values = np.nan)
    else:
        return np.empty(0, dtype = np.float32), np.empty(0, dtype = np.uint8)
    return np.array(data[:, feature_idx], dtype = np.float32), np.array(data[:, label_idx], dtype = np.uint8)

def get_accuracy(prediction, label):
    assert np.shape(prediction) == np.shape(label),  RuntimeError('Size not match')
    return sklearn.metrics.accuracy_score(label, prediction)
