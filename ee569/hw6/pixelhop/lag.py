# 2020.04.06 v3
# label assistant regression
# modified from Yueru
import numpy as np
from sklearn.cluster import MiniBatchKMeans, KMeans
from sklearn.metrics.pairwise import euclidean_distances

class LAG():
    def __init__(self, learner, encode='onehot', num_clusters=[10,10], alpha=5, par={}):
        # assert (str(learner.__class__) == "<class 'llsr.LLSR'>"), "Currently only support <class 'llsr.LLSR'>!"
        self.learner = learner
        self.encode = encode
        self.num_clusters = num_clusters
        self.alpha = alpha
        self.clus_labels = []
        self.centroid = []
        self.num_class = []
        self.trained = False

    def compute_target_(self, X, Y, batch_size):
        Y = Y.reshape(-1)
        class_list = np.unique(Y)
        labels = np.zeros((X.shape[0]))
        self.clus_labels = np.zeros((np.sum(self.num_clusters),))
        self.centroid = np.zeros((np.sum(self.num_clusters), X.shape[1]))
        start = 0
        for i in range(len(class_list)):
            ID = class_list[i]
            feature_train = X[Y==ID]
            if batch_size == None:
                kmeans = KMeans(n_clusters=self.num_clusters[i], verbose=0, random_state=9, n_jobs=10).fit(feature_train)
            else:
                kmeans = MiniBatchKMeans(n_clusters=self.num_clusters[i], verbose=0, batch_size=batch_size, n_init=5).fit(feature_train)
            labels[Y==ID] = kmeans.labels_ + start
            self.clus_labels[start:start+self.num_clusters[i]] = ID
            self.centroid[start:start+self.num_clusters[i]] = kmeans.cluster_centers_
            start += self.num_clusters[i]
        return labels.astype('int32')

    def fit(self, X, Y, batch_size=None):
        '''
        LAG unit: fit
        input: X of shape (N, K), N is the number of training samples
        '''
        self.num_class = len(np.unique(Y))
        assert (len(self.num_clusters) >= self.num_class), "'len(num_cluster)' must larger than class number!"
        Yt = self.compute_target_(X, Y, batch_size=batch_size)
        if self.encode == 'distance': # this is the mode used in the paper
            Yt_onehot = np.zeros((Yt.shape[0], self.clus_labels.shape[0]))
            for i in range(Yt.shape[0]):
                gt = Y[i].copy()
                dis = euclidean_distances(X[i].reshape(1,-1), self.centroid[self.clus_labels == gt]).reshape(-1)
                dis = dis / (np.min(dis) + 1e-15)
                p_dis = np.exp(-dis * self.alpha)
                p_dis = p_dis / np.sum(p_dis)
                Yt_onehot[i, self.clus_labels == gt] = p_dis
        elif self.encode == 'onehot':
            Yt_onehot = np.eye(len(np.unique(Yt)))[Yt.reshape(-1)]
        else:
            print("       <Warning>        Using raw label for learner.")
            Yt_onehot = Yt
        self.learner.fit(X, Yt_onehot)
        self.trained = True

    def transform(self, X):
        '''
        LAG unit: transformation
        input: X of shape (N, K)
        output: (N, M), M = sum(num_clusters[i]*num_class)
        Example: if having 10 classes, and create 5 seeds per class, output size = (N,50)
        '''
        assert (self.trained == True), "Must call fit first!"
        return self.learner.predict_proba(X)

    def predict_proba(self, X):
        '''
        LAG unit: group energy in each class
        input: X of shape (N, K)
        output: probability vectors of size(N, num_class)
        Example: if having 10 classes, output size = (N,10)
        '''
        assert (self.trained == True), "Must call fit first!"
        X = self.transform(X)
        pred_labels = np.zeros((X.shape[0], self.num_class))
        for km_i in range(self.num_class):
            pred_labels[:, km_i] = np.sum(X[:, self.clus_labels==km_i], axis=1)
        pred_labels = pred_labels/np.sum(pred_labels,axis=1,keepdims=1)
        return pred_labels

    def predict(self, X):
        '''
        LAG unit: group energy in each class
        input: X of shape (N, K)
        output: probability vectors of size(N, num_class)
        Example: if having 10 classes, output size = (N,10)
        '''
        return np.argmax(self.predict_proba(X), axis=1)

    def score(self, X, Y):
        assert (self.trained == True), "Must call fit first!"
        pred_labels = self.predict(X)
        idx = (pred_labels == Y.reshape(-1))
        return np.count_nonzero(idx) / Y.shape[0]

if __name__ == "__main__":
    from sklearn import datasets
    from sklearn.model_selection import train_test_split
    from llsr import LLSR as myLLSR

    print(" > This is a test example: ")
    digits = datasets.load_digits()
    X = digits.images.reshape((len(digits.images), -1))
    print(" input feature shape: %s"%str(X.shape))
    X_train, X_test, y_train, y_test = train_test_split(X, digits.target, test_size=0.2,  stratify=digits.target)

    lag = LAG(encode='distance', num_clusters=[2,2,2,2,2,2,2,2,2,2], alpha=5, learner=myLLSR(onehot=False))
    lag.fit(X_train, y_train)
    X_train_trans = lag.transform(X_train)
    X_train_predprob = lag.predict_proba(X_train)
    print(" --> train acc: %s"%str(lag.score(X_train, y_train)))
    print(" --> test acc.: %s"%str(lag.score(X_test, y_test)))
    print("------- DONE -------\n")
