#!/usr/bin/python3

from sklearn import preprocessing
from sklearn import linear_model
from sklearn import multiclass
import numpy as np
import argparse

from classifier import MSEBinaryClassifier

def load_csv(training_csv, test_csv):
    print('Training Data: %s' % training_csv)
    print('Test Data: %s' % test_csv)

    training_data = np.loadtxt(training_csv, delimiter = ',')
    test_data = np.loadtxt(test_csv, delimiter = ',')
    return (training_data, test_data)

def main(training_data, test_data):
    training_data_shape = np.shape(training_data)
    test_data_shape = np.shape(training_data)
    if (training_data_shape[1] != test_data_shape[1]):
        raise RuntimeError('Size of training and test data do not match')

    training_feature = training_data[:, :-1]
    training_label = training_data[:, -1]
    test_feature = test_data[:, :-1]
    test_label = test_data[:, -1]

    d = np.shape(training_feature)[1]
    c = np.size(np.unique(training_label))

    # (b)
    print("Part (b):")
    training_feature_mean = np.mean(training_feature, axis = 0)
    training_feature_stddev = np.std(training_feature, axis = 0)
    training_feature_std = (training_feature - training_feature_mean) / training_feature_stddev
    test_feature_std = (test_feature - training_feature_mean) / training_feature_stddev
    print("Mean: %s" % training_feature_mean)
    print("Standard Deviation: %s" % training_feature_stddev)
    print("================================\n")

    # (d)
    print("Part (d):")
    print("First Two Features:")
    perceptron = linear_model.Perceptron()
    perceptron.fit(training_feature_std[:, 0:2], training_label)
    print("Weight Vectors to Classes %s: \n%s" % \
        (perceptron.classes_, np.insert(perceptron.coef_, 0 , perceptron.intercept_, axis = 1)))
    train_score = perceptron.score(training_feature_std[:, 0:2], training_label)
    test_score = perceptron.score(test_feature_std[:, 0:2], test_label)
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (train_score, test_score))

    print("All Features:")
    perceptron.fit(training_feature_std, training_label)
    print("Weight Vectors to Classes %s: \n%s " % \
        (perceptron.classes_, np.insert(perceptron.coef_, 0 , perceptron.intercept_, axis = 1)))
    train_score = perceptron.score(training_feature_std, training_label)
    test_score = perceptron.score(test_feature_std, test_label)
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (train_score, test_score))
    print("================================\n")

    # (e)
    print("Part (e):")
    print("First Two Features:")
    max_score = 0
    for i in range(100):
        perceptron.fit(training_feature_std[:, 0:2], training_label, coef_init = np.random.random_sample((c,  2)))
        train_score = perceptron.score(training_feature_std[:, 0:2], training_label)
        if (train_score > max_score):
            coef = np.insert(perceptron.coef_, 0 , perceptron.intercept_, axis = 1)
            test_score = perceptron.score(test_feature_std[:, 0:2], test_label)
            max_score = train_score
    print("Weight Vectors to Classes %s: \n%s " % (perceptron.classes_, coef))
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (max_score, test_score))

    print("All Features:")
    for i in range(100):
        perceptron.fit(training_feature_std, training_label, coef_init = np.random.random_sample((c,  d)))
        train_score = perceptron.score(training_feature_std, training_label)
        if (train_score > max_score):
            coef = np.insert(perceptron.coef_, 0 , perceptron.intercept_, axis = 1)
            test_score = perceptron.score(test_feature_std, test_label)
            max_score = train_score
    print("Weight Vectors to Classes %s: \n%s " % (perceptron.classes_, coef))
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (max_score, test_score))
    print("================================\n")

    # (g)
    print("Part (g):")
    print("First Two Features:")
    mse_classifier = multiclass.OneVsRestClassifier(MSEBinaryClassifier())
    mse_classifier.fit(training_feature[:, 0:2], training_label)
    train_score = mse_classifier.score(training_feature[:, 0:2], training_label)
    test_score = mse_classifier.score(test_feature[:, 0:2], test_label)
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (train_score, test_score))

    print("All Features:")
    mse_classifier.fit(training_feature, training_label)
    train_score = mse_classifier.score(training_feature, training_label)
    test_score = mse_classifier.score(test_feature, test_label)
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (train_score, test_score))
    print("================================\n")

    # (h)
    print("Part (h):")
    print("First Two Features:")
    mse_classifier.fit(training_feature_std[:, 0:2], training_label)
    train_score = mse_classifier.score(training_feature_std[:, 0:2], training_label)
    test_score = mse_classifier.score(test_feature_std[:, 0:2], test_label)
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (train_score, test_score))

    print("All Features:")
    mse_classifier.fit(training_feature_std, training_label)
    train_score = mse_classifier.score(training_feature_std, training_label)
    test_score = mse_classifier.score(test_feature_std, test_label)
    print("Accuracy on Training Dataset: %f, on Test Dataset: %f" % (train_score, test_score))
    print("================================\n")

if (__name__ == '__main__'):
    parser = argparse.ArgumentParser()
    parser.add_argument('--train', help = 'training csv')
    parser.add_argument('--test', help = 'test csv')
    args = parser.parse_args()

    if (args.train and args.test):
        (training_data, test_data) = load_csv(args.train, args.test)
    else:
        print('Error: train and test csv is required but not specified')
        exit(1)

    np.set_printoptions(suppress=True)
    try:
        main(training_data, test_data)
    except RuntimeError as e:
        print("RuntimeError raised\n", e.args)
        exit(1)
