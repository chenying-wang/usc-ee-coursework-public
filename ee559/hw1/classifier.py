import numpy as np
from scipy.spatial.distance import cdist
import sys

from plotDecBoundaries import plotDecBoundaries

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

    feature_means = np.array([])
    classifications = np.array([])

    def __init__(self, d):
        super().__init__(d)

    def evaluate(self, feature):
        if (super().evaluate(feature) or np.size(self.feature_means) == 0):
            return False

        length = np.shape(feature)[0]
        min_distance = np.full(length, sys.maxsize)
        classification = np.full(length, -1)
        for i in range(self.c):
            distance = np.sum((feature - self.feature_means[i]) ** 2, axis = 1)
            classification = np.where(distance < min_distance, np.full(length, self.classifications[i]), classification)
            min_distance = np.where(distance < min_distance, distance, min_distance)
        return classification

    def train(self, data):
        data_feature = data[:, :-1]
        data_classfication = data[:, -1]
        if (np.shape(data_feature)[1] != self.d):
            raise RuntimeError("Size of training data does not fit")


        self.classifications = np.unique(data_classfication)
        self.c = len(self.classifications)
        self.feature_means = np.zeros((self.c, self.d))

        for i in range(self.c):
            feature_mean = np.mean(data_feature[data_classfication == self.classifications[i]], axis = 0)
            self.feature_means[i] = feature_mean
        return self
