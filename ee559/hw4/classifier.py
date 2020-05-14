import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial.distance import cdist
import sys

from plot_area import plot_area

COLOR = ['tab:blue', 'tab:orange', 'tab:green']

class BaseClassifier:

    d = -1
    c = -1

    def __init__(self, d):
        super().__init__()
        if (d <= 0):
            raise RuntimeError('Classifier.D/C cannot be negative or zero')
        self.d = d

    def evaluate(self, feature):
        if (np.shape(feature)[1] != self.d):
            raise RuntimeError('Size of feature does not fit')
        if (self.c < 0):
            raise RuntimeError('Evaluate with an untrained classifier')
        return 0

    def train(self, data):
        return self


class GradientDecentClassifier(BaseClassifier):

    training_features = np.array([])
    training_classifications = np.array([])
    classifications = np.array([])
    weights_group = np.array([])

    def __init__(self):
        super().__init__(2)

    def evaluate(self, feature):
        if (super().evaluate(feature) or np.size(self.weights_group) == 0):
            return None

        length = np.shape(feature)[0]
        classification = np.full(length, None)
        for i in range(self.c):
            weights = self.weights_group[i]
            result = np.matmul(weights, np.transpose(np.insert(feature, 0, 1, axis = 1)))
            classification[np.all(result > 0, axis = 0)] = self.classifications[i]
        return classification

    def train(self, data, epoch = 1000, learning_rate = 1):
        np.random.shuffle(data)
        self.training_features = np.array(data[:, :-1])
        self.training_classifications = np.array(data[:, -1])
        if (np.shape(self.training_features)[1] != self.d):
            raise RuntimeError('Size of training data does not fit')
        self.classifications = np.unique(self.training_classifications)
        self.c = np.size(self.classifications)

        if (self.c > 2):
            raise RuntimeError('Two-class supported only')


        self.reflect_features()
        data_size = np.shape(data)[0]
        weight = np.full(self.d + 1, 0.1)
        for i in range(epoch * data_size):
            training_features = self.training_features[i % data_size, :]
            training_classification = self.training_classifications[i % data_size]
            result = weight[0] + np.inner(weight[1:], training_features)
            if (result > 0):
                continue
            weight += learning_rate * np.insert(training_features, 0, 1)

        self.weights_group = np.array([[weight], [-weight]])
        self.reflect_features()
        return self

    def test(self, data):
        test_features = np.array(data[:, :-1])
        test_classifications = np.array(data[:, -1])
        classifications = self.evaluate(test_features)
        return np.count_nonzero(classifications - test_classifications) / \
            np.size(classifications)

    def reflect_features(self):
        if (np.size(self.training_features) == 0 or self.c < 0):
            raise RuntimeError('Reflect features with an untrained classifier')
        self.training_features \
            [self.training_classifications == self.classifications[1], :] *= -1

    def plot_features(self, ax):
        if (np.size(self.training_features) == 0 or self.c < 0):
            raise RuntimeError('Plot with an untrained classifier or not applicable')

        for i in range(self.c):
            features = self.training_features \
                [self.training_classifications == self.classifications[i], :]
            ax.scatter(features[:, 0], features[:, 1], \
                label = 'Class ' + str(int(self.classifications[i])), \
                marker = 'x', alpha = 0.8, color = COLOR[i])

    def plot_decision_regions_boundaries(self, ax):
        if (self.c < 0):
            raise RuntimeError('Plot with an untrained classifier')
        for i in range(self.c):
            plot_area(ax, [self.weights_group[i]], self.get_plot_linespace(), \
                label = 'Gamma_' + str(int(self.classifications[i])), \
                color = COLOR[i], alpha = 0.6)

    def get_plot_linespace(self):
        linespace_lim = self._get_plot_limit()[0]
        linespace = np.arange(linespace_lim[0], linespace_lim[1], 0.002)
        return linespace

    def plot_setlim(self, ax):
        plot_limit = self._get_plot_limit()
        ax.set_xlim(plot_limit[0][0], plot_limit[0][1])
        ax.set_ylim(plot_limit[1][0], plot_limit[1][1])

    def plot_legend(self, ax, **kwargs):
        handles, labels = ax.get_legend_handles_labels()
        newLabels, newHandles = [], []
        for handle, label in zip(handles, labels):
            if label not in newLabels:
                newLabels.append(label)
                newHandles.append(handle)
        ax.legend(newHandles, newLabels, **kwargs)

    def _get_plot_limit(self):
        if (np.size(self.training_features) == 0):
            return np.array([[0, 6], [0, 6]])
        min = np.min(self.training_features, axis = 0)
        max = np.max(self.training_features, axis = 0)
        min = min - np.maximum((max - min) * 0.25, [0.5, 0.5])
        max = max + np.maximum((max - min) * 0.25, [0.5, 0.5])
        return np.transpose([min, max])
