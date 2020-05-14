from sklearn import linear_model
import numpy as np

class MSEBinaryClassifier(linear_model.LinearRegression):

    def __init__(self):
        super().__init__()

    def predict(self, feature):
        y = super().predict(self.augment(feature))
        return np.where(y > 0, self.label[0], self.label[1])

    def fit(self, feature, label):
        self.label = np.unique(label)
        return super().fit(self.augment(feature), np.where(label == self.label[0], 1, -1))

    def augment(self, feature):
        return np.insert(feature, 0, np.ones(np.shape(feature)[0]), axis = 1)
