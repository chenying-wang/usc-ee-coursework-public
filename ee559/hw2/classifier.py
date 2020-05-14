import numpy as np
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
            raise RuntimeError("Classifier.D/C cannot be negative or zero")
        self.d = d

    def evaluate(self, feature):
        if (np.shape(feature)[1] != self.d):
            raise RuntimeError("Size of feature does not fit")
        if (self.c < 0):
            raise RuntimeError("Evaluate with an untrained classifier")
        return 0

    def train(self, data):
        return self


class NearestMeanClassifier(BaseClassifier):

    training_features = np.array([])
    training_classifications = np.array([])
    feature_means = np.array([])
    classifications = np.array([])
    num_of_classifications = np.array([])
    weights_group = np.array([])

    def __init__(self, d):
        super().__init__(d)

    def evaluate(self, feature):
        if (super().evaluate(feature) or np.size(self.weights_group) == 0):
            return None

        length = np.shape(feature)[0]
        min_distance = np.full(length, sys.maxsize)
        classification = np.full(length, None)
        for i in range(self.c):
            weights = self.weights_group[i]
            result = np.matmul(weights, np.transpose(np.insert(feature, 0, 1, axis = 1)))
            classification[np.all(result > 0, axis = 0)] = self.classifications[i]
        return classification

    def test(self, feature, classification):
        eval_classification = self.evaluate(feature)
        return np.count_nonzero(eval_classification == classification) / \
            np.size(classification)

    def train_by_weights_group(self, weights_group, classifications):
        self.weights_group = weights_group
        self.classifications = classifications
        self.c = len(self.classifications)

    def train(self, data, training_type = 'ovo'):
        self.training_features = data[:, :-1]
        self.training_classifications = data[:, -1]
        if (np.shape(self.training_features)[1] != self.d):
            raise RuntimeError("Size of training data does not fit")

        self.classifications = np.unique(self.training_classifications)
        self.c = len(self.classifications)
        self.feature_means = np.zeros([self.c, self.d])
        self.num_of_classifications, _ =  np.histogram(self.training_classifications, \
            bins = np.append(self.classifications, np.max(self.classifications) + 1))
        for i in range(self.c):
            self.feature_means[i] = np.mean(\
                self.training_features[self.training_classifications == self.classifications[i]], \
                axis = 0)

        if (training_type == 'ovo' or training_type == 'mvm'):
            weights_group = np.array([])
            for i in range(self.c):
                classification = self.classifications[i]
                weights = np.array([])
                for j in range(self.c):
                    if (i == j):
                        continue
                    another_classification = self.classifications[j]
                    weight = self.get_boundary_weight_by_ovo(classification, another_classification)
                    if (np.size(weights) == 0):
                        weights = np.array([weight])
                    else:
                        weights = np.append(weights, [weight], axis = 0)
                if (np.size(weights_group) == 0):
                    weights_group = np.array([weights])
                else:
                    weights_group = np.append(weights_group, [weights], axis = 0)
            self.weights_group = weights_group
        elif (training_type == 'ovr'):
            weights = np.array([])
            for classification in self.classifications:
                weight = self.get_boundary_weight_by_ovr(classification)
                if (np.size(weights) == 0):
                    weights = np.array([weight])
                else:
                    weights = np.append(weights, [weight], axis = 0)

            weights_group = np.tile(-weights, (self.c, 1, 1))
            for i in range(self.c):
                weights_group[i][i] = -weights_group[i][i]
            self.weights_group = weights_group
        return self

    def get_boundary_weight_by_ovo(self, classification, another_classification):
        if (self.c < 0):
            raise RuntimeError("Get boundary with an untrained classifier")

        return self._get_boundary_weight_by_two_means(\
            self.feature_means[self.classifications == classification], \
            self.feature_means[self.classifications == another_classification])

    def get_boundary_weight_by_ovr(self, classification):
        if (self.c < 0):
            raise RuntimeError("Get boundary with an untrained classifier")

        i = np.argwhere(self.classifications == classification)
        feature_mean = self.feature_means[i]
        non_feature_means = np.delete(self.feature_means, i, axis = 0)
        num_of_non_classifications = np.delete(self.num_of_classifications, i)
        non_feature_mean = np.average(non_feature_means, weights = num_of_non_classifications, axis = 0)

        return self._get_boundary_weight_by_two_means(feature_mean, non_feature_mean)

    def plot_features(self, ax):
        if (np.size(self.training_features) == 0 or self.c < 0):
            raise RuntimeError('Plot with an untrained classifier or not applicable')

        for i in range(self.c):
            features = self.training_features[self.training_classifications == self.classifications[i], :]
            ax.scatter(features[:, 0], features[:, 1], label = 'Class ' + str(int(self.classifications[i])), \
                marker = 'x', alpha = 0.8, color = COLOR[i])

    def plot_means(self, ax):
        if (np.size(self.feature_means) == 0 or self.c < 0):
            raise RuntimeError('Plot with an untrained classifier or not applicable')

        for i in range(self.c):
            ax.scatter(self.feature_means[i][0], self.feature_means[i][1], label = 'Class ' + str(self.classifications[i]) + ' Mean', \
                linewidth = 8, marker = 'o', color = COLOR[i])

    def plot_decision_regions_boundaries(self, ax):
        if (self.c < 0):
            raise RuntimeError('Plot with an untrained classifier')
        for i in range(self.c):
            plot_area(ax, [self.weights_group[i]], self.get_plot_linespace(), \
                label = 'Gamma_' + str(int(self.classifications[i])), \
                color = COLOR[i], alpha = 0.6)

    def plot_indeterminate_regions(self, ax):
        if (self.c < 0):
            raise RuntimeError('Plot with an untrained classifier')

        indeterminate_weights_group = self._get_complement_weights_group(self.weights_group)
        plot_area(ax, indeterminate_weights_group, self.get_plot_linespace(), \
            label = 'Indeterminate', \
            color = 'tab:grey')

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

    def _get_boundary_weight_by_two_means(self, a, b):
        return np.append(np.sum(np.square(b)) - np.sum(np.square(a)), 2 * (a - b))

    def _get_complement_weights_group(self, weights_group, complement_weights_group = np.array([]), weights = np.array([])):
        _weights = weights
        for weight in weights_group[0]:
            if (np.size(_weights) == 0):
                weights = np.array([-weight])
            else:
                weights = np.append(_weights, [-weight], axis = 0)
            if (np.shape(weights_group)[0] > 1):
                complement_weights_group = self._get_complement_weights_group(weights_group = weights_group[1:, :, :], \
                    complement_weights_group = complement_weights_group,
                    weights = weights)
            else:
                if (np.size(complement_weights_group) == 0):
                    complement_weights_group = np.array([weights])
                else:
                    complement_weights_group = np.append(complement_weights_group, [weights], axis = 0)
        return complement_weights_group
