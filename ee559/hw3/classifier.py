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


class AugmentedSpaceClassifier(BaseClassifier):

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

    def train_by_weights_group(self, weights_group, classifications):
        self.weights_group = weights_group
        self.classifications = classifications
        self.c = len(self.classifications)

    def train(self, data):
        self.training_features = data[:, :-1]
        self.training_classifications = data[:, -1]
        if (np.shape(self.training_features)[1] != self.d):
            raise RuntimeError('Size of training data does not fit')

        self.classifications = np.unique(self.training_classifications)
        self.c = len(self.classifications)

        feature_min = np.array([0, 0])
        feature_max = np.array([0, 0])
        for i in range(self.c):
            feature_min[i] = np.min(\
                self.training_features[self.training_classifications == self.classifications[i], 1], \
                axis = 0)
            feature_max[i] = np.max(\
                self.training_features[self.training_classifications == self.classifications[i], 1], \
                axis = 0)
        if (feature_min[self.classifications == 1] > feature_max[self.classifications == 2]):
            weights_group = np.array([[[0, \
                -(feature_min[self.classifications == 1] + feature_max[self.classifications == 2])[0] / 2, 1]], \
                [[0, (feature_min[self.classifications == 1] + feature_max[self.classifications == 2])[0] / 2, -1]]])
        elif (feature_max[self.classifications == 1] < feature_min[self.classifications == 2]):
            weights_group = np.array([[[0, \
                (feature_max[self.classifications == 1] + feature_min[self.classifications == 2])[0] / 2, -1]], \
                [[0, -(feature_max[self.classifications == 1] + feature_min[self.classifications == 2])[0] / 2, 1]]])
        else:
            raise RuntimeError('Failed to train classifier')
        self.weights_group = weights_group
        return self

    def reflect_features(self):
        if (np.size(self.training_features) == 0 or self.c < 0):
            raise RuntimeError('Reflect features with an untrained classifier')

        self.training_features[self.training_classifications == 2, :] *= -1

    def plot_features(self, ax):
        if (np.size(self.training_features) == 0 or self.c < 0):
            raise RuntimeError('Plot with an untrained classifier or not applicable')

        for i in range(self.c):
            features = self.training_features[self.training_classifications == self.classifications[i], :]
            ax.scatter(features[:, 0], features[:, 1], label = 'Class ' + str(self.classifications[i]), \
                marker = 'x', alpha = 0.8, color = COLOR[i])

    def plot_positive_decision_regions_boundaries(self, ax):
        if (self.c < 0):
            raise RuntimeError('Plot with an untrained classifier')
        plot_area(ax, self.weights_group[self.classifications == 1], self.get_plot_linespace(), \
            label = 'Positive', color = COLOR[0], alpha = 0.6)

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

    def plot_features_in_weight_space(self, ax):
        if (np.size(self.training_features) == 0 or self.c < 0):
            raise RuntimeError('Plot with an untrained classifier or not applicable')

        weights = np.array([])
        for features in self.training_features:
            weight = np.array([0])
            weight = np.append(weight, features)
            if (np.size(weights) == 0):
                weights = np.array([weight])
            else:
                weights = np.append(weights, [weight], axis = 0)
            plot_area(ax, [[weight]], self.get_plot_linespace_in_weight_space(), \
                label = 'Positive', color = COLOR[0], alpha = 0.6)
        plot_area(ax, [weights], self.get_plot_linespace_in_weight_space(), \
                label = 'Solution', hatch = '/', color = COLOR[1], alpha = 0.6)

    def plot_weight_vector_in_weight_space(self, ax):
        weight_vector = self.weights_group[self.classifications == 1, 0, 1:3][0]
        ax.arrow(0, 0, weight_vector[0], weight_vector[1], label = 'Weight Vector', \
            head_width = 0.1, head_length = 0.2, color = 'black', alpha = 0.8)

    def get_plot_linespace_in_weight_space(self):
        return np.arange(-3, 3, 0.002)

    def plot_setlim_in_weight_space(self, ax):
        ax.set_xlim(-3, 3)
        ax.set_ylim(-3, 3)

    def _get_plot_limit(self):
        if (np.size(self.training_features) == 0):
            return np.array([[0, 6], [0, 6]])
        min = np.min(self.training_features, axis = 0)
        max = np.max(self.training_features, axis = 0)
        min = min - np.maximum((max - min) * 0.25, [0.5, 0.5])
        max = max + np.maximum((max - min) * 0.25, [0.5, 0.5])
        return np.transpose([min, max])
