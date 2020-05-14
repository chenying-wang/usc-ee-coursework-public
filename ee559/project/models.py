import sklearn
import sklearn.base
import sklearn.decomposition
import sklearn.ensemble
import sklearn.metrics
import sklearn.model_selection
import sklearn.naive_bayes
import sklearn.neighbors
import sklearn.svm
import numpy as np
import matplotlib.pyplot as plt

import classifiers
import utils

class EstimatorBase(
    sklearn.base.BaseEstimator,
    sklearn.base.ClassifierMixin):

    def __init__(self, clf = 'bayes', knn_n_neighbors = 5, \
        svm_c = 1, svm_gamma = 'scale', \
        nn_validation = False):
        self.clf = clf
        self.knn_n_neighbors = knn_n_neighbors
        self.svm_c = svm_c
        self.svm_gamma = svm_gamma
        self.nn_validation = nn_validation

        if clf == 'bayes' or clf == None:
            self._clf_instance = sklearn.naive_bayes.GaussianNB()
        elif clf == 'knn':
            self._clf_instance = sklearn.neighbors.KNeighborsClassifier(n_neighbors = knn_n_neighbors, n_jobs = -1)
        elif clf == 'svm':
            self._clf_instance = sklearn.svm.SVC(C = svm_c, gamma = svm_gamma)
        elif clf == 'nn':
            self._clf_instance = classifiers.NeuralNetwork([48, 64, 64, 5], validation = nn_validation)
        elif clf == 'ensemble':
            self._clf_instance = classifiers.EmsembleClassifer(C = svm_c, gamma = svm_gamma)
        else:
            raise RuntimeError('Unknown classifier')
        self._fitted = False

    def fit(self, x, y):
        self._fitted = False
        self._label = np.unique(y)
        self._fitted = True
        return self

    def predict(self, x):
        assert self._fitted, 'Model is not trained'
        return self._label[np.random.randint(len(self._label), size = len(x))]

    def evaluate(self, x, y, cm = None):
        y_pred = self.predict(x)
        if cm == None:
            return utils.get_accuracy(y_pred, y)
        labels = np.unique(y)
        confusion_matrix = sklearn.metrics.confusion_matrix(y, y_pred, labels, normalize = 'true')
        fig, ax = plt.subplots()
        fig.set_size_inches(6, 6)
        sklearn.metrics.ConfusionMatrixDisplay(confusion_matrix, labels).plot(ax = ax, cmap = 'YlOrBr', values_format = '.2%')
        plt.savefig(cm, dpi = 200)
        fig.clf()
        return utils.get_accuracy(y_pred, y)

    def cross_validate(self, x, y):
        if self.clf == 'nn':
            return
        cv_splitter = sklearn.model_selection.StratifiedKFold(n_splits = 5, shuffle = True)
        return sklearn.model_selection.cross_val_score(self, x, y, cv = cv_splitter, n_jobs = None)


class PosturesEstimator(EstimatorBase):

    def __init__(self, clf = 'bayes',  aggregation = False, knn_n_neighbors = 5, \
        svm_c = 1, svm_gamma = 'scale', \
        nn_validation = False):
        super(PosturesEstimator, self).__init__(clf, knn_n_neighbors, svm_c, svm_gamma, nn_validation)
        self.aggregation = aggregation

    def fit(self, x, y):
        assert len(x) == len(y), 'Data size not match'
        self._fitted = False
        user, x = self._preprocess(x)
        if self.aggregation:
            x = self._aggregate_features(x)
        else:
            x = self._fix_raw_features(x)
        if self.clf == 'ensemble' or self.clf == 'nn':
            self._clf_instance.fit(x, y, user = user)
        else:
            self._clf_instance.fit(x, y)
        return super().fit(x, y)

    def predict(self, x):
        assert self._fitted, 'Model is not trained'
        user, x = self._preprocess(x)
        if self.aggregation:
            x = self._aggregate_features(x)
        else:
            x = self._fix_raw_features(x)
        return self._clf_instance.predict(x)

    def _preprocess(self, x):
        assert len(np.shape(x)) == 2, 'Two-dimension feature expected'
        x_user, x_features = x[:, 0], x[:, 1:]
        x_features = np.reshape(x_features, (len(x), -1, 3))
        if not self._fitted:
            self._x_mean = np.nanmean(x_features, axis = (0, 1))
            self._x_stddev = np.nanstd(x_features, axis = (0, 1), ddof = 1)
        x_features_std = (x_features - self._x_mean) / self._x_stddev
        return x_user, x_features_std

    def _fix_raw_features(self, x):
        assert len(np.shape(x)) == 3, 'Three-dimension feature expected'
        return np.reshape(np.nan_to_num(x), (len(x), -1))

    def _aggregate_features(self, x):
        x_num = np.expand_dims(np.count_nonzero(np.isfinite(x), axis = (1, 2)) / 3, axis = 1)
        x_max = np.nanmax(x, axis = 1)
        x_min = np.nanmin(x, axis = 1)
        x_mean = np.nanmean(x, axis = 1)
        x_stddev = np.nanstd(x, axis = 1)

        x_pca = np.empty((len(x), 3), dtype = np.float32)
        pca = sklearn.decomposition.PCA(n_components = 1)
        for i in range(len(x)):
            x_pca[i] = np.reshape(pca.fit(x[i, :int(x_num[i, 0])]).components_, -1)

        x_distance_distribution = np.empty((len(x), 4), dtype = np.float32)
        for i in range(len(x)):
            distance = np.linalg.norm(x[i, :int(x_num[i, 0])] - x_mean[i], axis = 1)
            x_distance_distribution[i] = [np.max(distance), np.min(distance), np.mean(distance), np.std(distance)]
        return np.hstack((x_num, x_max, x_min, x_mean, x_stddev, x_pca, x_distance_distribution))

    def cross_validate(self, x, y):
        if self.clf == 'nn':
            return
        cv_splitter = sklearn.model_selection.LeavePGroupsOut(1)
        return sklearn.model_selection.cross_val_score(self, x, y, groups = x[:, 0], cv = cv_splitter, n_jobs = -1)
