#v2020.03.19v3
import numpy as np
from sklearn.metrics import accuracy_score
class LLSR():
    def __init__(self, onehot=True, normalize=False):
        self.onehot = onehot
        self.normalize = normalize
        self.weight = []
        self.trained = False

    def fit(self, X, Y):
        if self.onehot == True:
            Y = np.eye(len(np.unique(Y)))[Y.reshape(-1)]
        A = np.ones((X.shape[0], 1))
        X = np.concatenate((X, A), axis=1)
        self.weight, _, _, _ = np.linalg.lstsq(X, Y, rcond=None)
        self.trained = True

    def predict(self, X):
        assert (self.trained == True), "Must call fit first!"
        pred = self.predict_proba(X)
        return np.argmax(pred, axis=1)

    def predict_proba(self, X):
        assert (self.trained == True), "Must call fit first!"
        A = np.ones((X.shape[0], 1))
        X = np.concatenate((X, A), axis=1)
        pred = np.matmul(X, self.weight)
        if self.normalize == True:
            pred = (pred - np.min(pred, axis=1, keepdims=True))/ np.sum((pred - np.min(pred, axis=1, keepdims=True) + 1e-15), axis=1, keepdims=True)
        return pred

    def score(self, X, Y):
        assert (self.trained == True), "Must call fit first!"
        pred = self.predict(X)
        return accuracy_score(Y, pred)
    
if __name__ == "__main__":
    from sklearn import datasets
    from sklearn.model_selection import train_test_split
    
    print(" > This is a test example: ")
    digits = datasets.load_digits()
    X = digits.images.reshape((len(digits.images), -1))
    print(" input feature shape: %s"%str(X.shape))
    X_train, X_test, y_train, y_test = train_test_split(X, digits.target, test_size=0.2, stratify=digits.target)
    
    reg = LLSR(onehot=True, normalize=False)
    reg.fit(X_train, y_train)
    X_train_reg = reg.predict_proba(X_train)
    print(" --> train acc: %s"%str(reg.score(X_train, y_train)))
    print(" --> test acc: %s"%str(reg.score(X_test, y_test)))
    print("------- DONE -------\n")
