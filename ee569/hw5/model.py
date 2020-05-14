# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, April 3, 2020

import tensorflow as tf
import argparse

import config

class BaseModel:

    def __init__(self, inputs_shape, lr_rate = config.SGD_LEARNING_RATE, lr_decay_steps = config.SGD_LEARNING_RATE_DECAY_STEPS, \
        lr_decay_rate = config.SGD_LEARNING_RATE_DECAY_RATE, name = 'model', **kwargs):
        self._build_model(inputs_shape, lr_rate, lr_decay_steps, lr_decay_rate, name)

    def fit(self, *args, **kwargs):
        self._model.fit(*args, **kwargs)

    def evaluate(self, *args, **kwargs):
        self._model.evaluate(*args, **kwargs)

    def load_weights(self, *args, **kwargs):
        self._model.load_weights(*args, **kwargs)

    def _build_model(self, inputs_shape, lr_rate, lr_decay_steps, lr_decay_rate, name):
        inputs = tf.keras.Input(inputs_shape)
        outputs = self.call(inputs)
        self._model = tf.keras.Model(inputs = inputs, outputs = outputs, name = name)
        self._model.summary()

        if lr_decay_steps <= 0:
            optimizer = tf.keras.optimizers.SGD(learning_rate = lr_rate, momentum = 0.9)
        else:
            lr_schedule = tf.keras.optimizers.schedules.ExponentialDecay(lr_rate, \
                decay_steps = lr_decay_steps * config.TRAIN_SIZE // config.BATCH_SIZE, \
                decay_rate = lr_decay_rate, staircase = True)
            optimizer = tf.keras.optimizers.SGD(learning_rate = lr_schedule, momentum = 0.9)
        self._model.compile(optimizer = optimizer, \
                            loss = tf.keras.losses.SparseCategoricalCrossentropy(), \
                            metrics = ['accuracy'])


class LeNet5Model(BaseModel):

    def __init__(self, inputs_shape, lr_rate = config.SGD_LEARNING_RATE, lr_decay_steps = config.SGD_LEARNING_RATE_DECAY_STEPS, \
        lr_decay_rate = config.SGD_LEARNING_RATE_DECAY_RATE, name = 'model', **kwargs):
        self.conv_1 = tf.keras.layers.Conv2D(6, (5, 5), padding = 'valid', \
            input_shape = (config.CHANNEL, config.HEIGHT, config.WIDTH), \
            data_format = 'channels_first', name = 'Conv_1')
        self.pool_1 = tf.keras.layers.MaxPool2D((2, 2), data_format = 'channels_first', name = 'Pool_1')
        self.conv_2 = tf.keras.layers.Conv2D(16, (5, 5), padding = 'valid', \
            data_format = 'channels_first', name = 'Conv_2')
        self.pool_2 = tf.keras.layers.MaxPool2D((2, 2), data_format = 'channels_first', name = 'Pool_2')
        self.flatten = tf.keras.layers.Flatten(name = 'Flatten')
        self.fc_1 = tf.keras.layers.Dense(120, activation = tf.nn.swish, name = 'Fc_1')
        self.dropout_1 = tf.keras.layers.Dropout(config.DROPOUT_RATE, name = 'Dropout_1')
        self.fc_2 = tf.keras.layers.Dense(84, activation = tf.nn.swish, name = 'Fc_2')
        self.dropout_2 = tf.keras.layers.Dropout(config.DROPOUT_RATE, name = 'Dropout_2')
        self.fc_3 = tf.keras.layers.Dense(10, activation = tf.nn.softmax, name = 'Prediction')
        super(LeNet5Model, self).__init__(inputs_shape, lr_rate, lr_decay_steps, lr_decay_rate, name, **kwargs)

    def call(self, inputs, training = None):
        x = self.conv_1(inputs)
        x = tf.nn.relu(x)
        x = self.pool_1(x)
        x = self.conv_2(x)
        x = tf.nn.relu(x)
        x = self.pool_2(x)
        x = self.flatten(x)
        x = self.fc_1(x)
        x = self.fc_2(x)
        prediction = self.fc_3(x)
        return prediction


class ModifiedLeNet5Model(BaseModel):

    def __init__(self, inputs_shape, lr_rate = config.SGD_LEARNING_RATE, lr_decay_steps = config.SGD_LEARNING_RATE_DECAY_STEPS, \
        lr_decay_rate = config.SGD_LEARNING_RATE_DECAY_RATE, name = 'model', **kwargs):
        self.conv_1 = tf.keras.layers.Conv2D(32, (5, 5), padding = 'valid', \
            kernel_regularizer = tf.keras.regularizers.l2(config.WEIGHT_DECAY), \
            input_shape = (config.CHANNEL, config.HEIGHT, config.WIDTH), \
            data_format = 'channels_first', name = 'Conv_1')
        self.bn_1 = tf.keras.layers.BatchNormalization(axis = 1, name = 'BN_1')
        self.pool_1 = tf.keras.layers.MaxPool2D((2, 2), data_format = 'channels_first', name = 'Pool_1')
        self.conv_2 = tf.keras.layers.Conv2D(64, (5, 5), padding = 'valid', \
            kernel_regularizer = tf.keras.regularizers.l2(config.WEIGHT_DECAY), \
            data_format = 'channels_first', name = 'Conv_2')
        self.bn_2 = tf.keras.layers.BatchNormalization(axis = 1, name = 'BN_2')
        self.pool_2 = tf.keras.layers.MaxPool2D((2, 2), data_format = 'channels_first', name = 'Pool_2')
        self.flatten = tf.keras.layers.Flatten(name = 'Flatten')
        self.fc_1 = tf.keras.layers.Dense(200, activation = tf.nn.swish, name = 'Fc_1')
        self.dropout_1 = tf.keras.layers.Dropout(config.DROPOUT_RATE, name = 'Dropout_1')
        self.fc_2 = tf.keras.layers.Dense(100, activation = tf.nn.swish, name = 'Fc_2')
        self.dropout_2 = tf.keras.layers.Dropout(config.DROPOUT_RATE, name = 'Dropout_2')
        self.fc_3 = tf.keras.layers.Dense(10, activation = tf.nn.softmax, name = 'Prediction')
        super(ModifiedLeNet5Model, self).__init__(inputs_shape, lr_rate, lr_decay_steps, lr_decay_rate, name, **kwargs)

    def call(self, inputs, training = None):
        x = self.conv_1(inputs)
        x = self.bn_1(x, training = training)
        x = tf.nn.swish(x)
        x = self.pool_1(x)
        x = self.conv_2(x)
        x = self.bn_2(x, training = training)
        x = tf.nn.swish(x)
        x = self.pool_2(x)
        x = self.flatten(x)
        x = self.fc_1(x)
        x = self.dropout_1(x, training = training)
        x = self.fc_2(x)
        x = self.dropout_2(x, training = training)
        prediction = self.fc_3(x)
        return prediction
