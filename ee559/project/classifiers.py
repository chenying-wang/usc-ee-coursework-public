import tensorflow as tf
import sklearn
import sklearn.base
import sklearn.ensemble
import sklearn.svm
import numpy as np

import logging

class NeuralNetwork(
    tf.keras.Model,
    sklearn.base.ClassifierMixin):

    def __init__(self, channel_list, validation = False):
        super(NeuralNetwork, self).__init__()
        self.channel_list = channel_list
        self.validation = validation

        tf.get_logger().setLevel(logging.WARNING)
        tf.random.set_seed(42)
        self._dense_layers = []
        for i in range(len(channel_list) - 1):
            self._dense_layers.append(tf.keras.layers.Dense(channel_list[i], activation = tf.nn.swish, \
                kernel_regularizer = tf.keras.regularizers.l2(1e-4), name = 'dense_' + str(i)))
        self._dense_layers.append(tf.keras.layers.Dense(channel_list[len(channel_list) - 1], activation = tf.nn.softmax, name = 'softmax'))

    def call(self, inputs):
        x = inputs
        for dense in self._dense_layers:
            x = dense(x)
        return x

    def fit(self, x, y, user = [], batch_size = 32, epochs = 200):
        self._label = np.unique(y)
        _dict = {}
        for i in range(len(self._label)):
            _dict[self._label[i]] = i
        _y = []
        for i in range(len(y)):
            _y.append(_dict[y[i]])
        _y = np.array(_y, dtype = np.uint8)
        lr = tf.keras.optimizers.schedules.ExponentialDecay(1e-3, 60 * len(x), 0.5, staircase = True)
        self.compile(optimizer = tf.keras.optimizers.SGD(learning_rate = lr, momentum = 0.9), \
            loss = tf.keras.losses.SparseCategoricalCrossentropy(from_logits = True), \
            metrics = ['accuracy'])
        self.build(input_shape = (None, np.shape(x)[1]))
        self.summary()

        callbacks = [
            tf.keras.callbacks.CSVLogger('nn_history.csv')
        ]
        if self.validation and len(user) == len(x):
            user_group = np.unique(user)
            np.random.shuffle(user_group)
            x_train = x[user != user_group[0]]
            _y_train = _y[user != user_group[0]]
            x_val = x[user == user_group[0]]
            _y_val = _y[user == user_group[0]]
            super(NeuralNetwork, self).fit(x_train, _y_train, validation_data = (x_val, _y_val), \
                    batch_size = batch_size, epochs = epochs, callbacks = callbacks)
        else:
            super(NeuralNetwork, self).fit(x, _y, \
                batch_size = batch_size, epochs = epochs, callbacks = callbacks)
        return self

    def predict(self, x, batch_size = 32):
        _y_pred_prob = super(NeuralNetwork, self).predict(x, batch_size = batch_size)
        _y_pred = np.argmax(_y_pred_prob, axis = 1)
        return self._label[_y_pred]
