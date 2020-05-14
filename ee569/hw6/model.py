# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, April 24, 2020

import time
import concurrent
import multiprocessing
import multiprocessing.managers
import os
import logging

import sklearn
import sklearn.ensemble
import skimage
import numpy as np
import matplotlib.pyplot as plt

import pixelhop

CIFAR10_LABELS = ['airplane', 'automobile', 'bird', 'cat', 'deer', 'dog', 'frog', 'horse', 'ship', 'truck']

def _bin_process_loss(ce, x, y):
    mc = ce.bin_process(x, y)
    l, c = np.unique(y, return_counts = True)
    count = dict(zip(l, c))
    l, p = np.unique(mc, return_counts = True)
    p = p / len(mc)
    prob = dict(zip(l, p))
    loss = 0
    for label in count.keys():
        p = prob[label] if label in prob else 0
        loss -= count[label] * np.log(p + 1e-6)
    return loss

class PixelHopPP:

    _logger = logging.getLogger('PixelHopPP')
    _logger.setLevel(logging.DEBUG)

    _logger_ch = logging.StreamHandler()
    _logger_ch.setLevel(logging.INFO)
    _logger_ch.setFormatter(logging.Formatter('[%(asctime)s] [%(levelname)s] [%(name)s] %(message)s'))
    _logger.addHandler(_logger_ch)

    _logger_fh = logging.FileHandler('debug.log')
    _logger_fh.setLevel(logging.DEBUG)
    _logger_fh.setFormatter(logging.Formatter('[%(asctime)s] [%(levelname)s] [%(name)s] %(message)s'))
    _logger.addHandler(_logger_fh)

    def __init__(self, data_n_batch = 500, data_n_fit_batch = 500, \
            data_n_test_batch = 100, \
            pixelhop_n_fit_batch = 100, pixelhop_neighborhood_win = 5, pixelhop_depth = 3, \
            pixelhop_th1 = 1e-3, pixelhop_th2 = 1e-4, \
            pixelhop_n_ac_kernels = [10, 8, 6], \
            ce_n_features = 1000, ce_n_centroids = 30, \
            lag_n_centroids = 5, lag_alpha = 10, rf_min_samples_leaf = 5):
        self._train = False
        self._data_n_batch = data_n_batch
        self._data_n_fit_batch = min(data_n_fit_batch, data_n_batch)
        self._data_n_test_batch = data_n_test_batch
        self._pixelhop_n_fit_batch = min(pixelhop_n_fit_batch, data_n_batch)
        self._pixelhop_depth = pixelhop_depth
        self._pixelhop_neighborhood_win = pixelhop_neighborhood_win
        self._pixelhop_th1 = pixelhop_th1
        self._pixelhop_th2 = pixelhop_th2
        self._pixelhop_n_ac_kernels = pixelhop_n_ac_kernels
        self._ce_n_features = ce_n_features
        self._ce_n_centroids = ce_n_centroids
        self._lag_n_centroids = lag_n_centroids
        self._lag_alpha = lag_alpha
        self._rf_min_samples_leaf = rf_min_samples_leaf

        self._saab_args = [{'num_AC_kernels': self._pixelhop_n_ac_kernels[0], 'needBias': False, 'useDC': True, 'cw': True}, \
                           {'num_AC_kernels': self._pixelhop_n_ac_kernels[1], 'needBias': True, 'useDC': True, 'cw': True}, \
                           {'num_AC_kernels': self._pixelhop_n_ac_kernels[2], 'needBias': True, 'useDC': True, 'cw': True}]
        self._shrink_args = [{'func': self._shrink, 'neighborhood_win': self._pixelhop_neighborhood_win, 'neighborhood_stride': 1, 'pool_win': 1, 'pool_stride': 1}, \
                             {'func': self._shrink, 'neighborhood_win': self._pixelhop_neighborhood_win, 'neighborhood_stride': 1, 'pool_win': 2, 'pool_stride': 2}, \
                             {'func': self._shrink, 'neighborhood_win': self._pixelhop_neighborhood_win, 'neighborhood_stride': 1, 'pool_win': 2, 'pool_stride': 2}, \
                             {'func': self._shrink, 'neighborhood_win': 1, 'neighborhood_stride': 1, 'pool_win': 1, 'pool_stride': 1}]
        self._concat_args = {'func': lambda X, concatArg: X}

        self._ce_selected_idx = []
        self._lags = []
        self._final_classifier = None

        self._manager = multiprocessing.managers.BaseManager()
        self._executor = concurrent.futures.ProcessPoolExecutor(max_workers = os.cpu_count() + 4)

    def __del__(self):
        self._executor.shutdown()
        self._manager.shutdown()

    def fit(self, x, y):
        self._logger.info('Fit Start')
        start = time.time()
        self._call(x, y, train = True)
        elapsed = time.time() - start
        self._logger.info('Fit in %.3fs (%d:%d)' % (elapsed, elapsed // 60, elapsed % 60))

    def predict(self, x):
        self._logger.info('Predict Start')
        start = time.time()
        y_predict = self._call(x)
        elapsed = time.time() - start
        self._logger.info('Predict in %.3fs (%d:%d)' % (elapsed, elapsed // 60, elapsed % 60))
        return y_predict

    def evaluate(self, x, y):
        y_predict = self.predict(x)
        error_rate = np.count_nonzero(y_predict - y) / len(y)
        self._logger.info('Error Rate on Test Dataset: %.6f' % error_rate)
        confusion_matrix = sklearn.metrics.confusion_matrix(y, y_predict, normalize = 'true')
        fig, ax = plt.subplots()
        fig.set_size_inches(12, 12)
        fig.set_dpi(600)
        sklearn.metrics.ConfusionMatrixDisplay(confusion_matrix, CIFAR10_LABELS).plot(ax = ax, values_format = '.2%')
        plt.savefig('./confusion.png')

    def summary(self):
        assert(self._train), 'Model is untrained'
        total_size = 0
        energy = self._manager.ph2_energy().copy()
        for i in range(len(energy)):
            shape  = (self._pixelhop_neighborhood_win, self._pixelhop_neighborhood_win, \
                np.count_nonzero(energy[i] >= self._pixelhop_th2))
            size = np.prod(shape)
            total_size += size
            self._logger.info('PixelHop Unit %d Size %s %d' % (i, shape, size))
        for i in range(len(self._lags)):
            lag = self._lags[i]
            size = np.size(lag.learner.weight)
            total_size += size
            self._logger.info('LAG Unit %d Size %s %d' % (i, np.shape(lag.learner.weight), size))
        self._logger.info('Total Size: %d' % total_size)

    def _call(self, x, y = None, train = False):
        if train:
            self._train = False
            data_batch = self._batch_dataset(x, y, self._data_n_batch, True)
            self._data_n_class = len(np.unique(data_batch[0]['label']))
            _ph2 = pixelhop.Pixelhop2(depth = self._pixelhop_depth, TH1 = self._pixelhop_th1, TH2 = self._pixelhop_th2, \
                SaabArgs = self._saab_args, shrinkArgs = self._shrink_args, concatArg = self._concat_args)
            self._manager.register('ph2', lambda: _ph2)
            self._manager.register('ph2_energy', lambda: _ph2.Energy)
            self._manager.start()
            self._ce_selected_idx.clear()
            self._lags.clear()
            self._final_classifier = sklearn.ensemble.RandomForestClassifier( \
                min_samples_leaf = self._rf_min_samples_leaf, n_jobs = -1)
            for i in range(self._pixelhop_depth):
                lag = pixelhop.LAG(encode = 'distance', num_clusters = np.full(self._data_n_class, self._lag_n_centroids), \
                        alpha = self._lag_alpha, learner = pixelhop.LLSR(onehot = False))
                self._lags.append(lag)
        else:
            assert(self._train), 'Model is untrained'
            data_batch = self._batch_dataset(x, None, self._data_n_test_batch, False)

        if train:
            x_ph2_train = data_batch[0]['feature']
            for i in range(1, self._pixelhop_n_fit_batch):
                x_ph2_train = np.concatenate((x_ph2_train, data_batch[i]['feature']), axis = 0)
            self._manager.ph2().fit(x_ph2_train)
            del x_ph2_train
            self._logger.info('PixelHop Fit Done')

        shrink_args = []
        for i in range(self._pixelhop_depth):
            shrink_args.append(self._shrink_args[i + 1].copy())
            shrink_args[i]['neighborhood_win'] = 1
            shrink_args[i]['neighborhood_stride'] = 1

        n_batch = self._data_n_fit_batch if train else self._data_n_test_batch

        futures = []
        for i in range(n_batch):
            future = self._executor.submit(self._manager.ph2().transform, data_batch[i]['feature'])
            futures.append(future)

        for i in range(n_batch):
            if i == 0:
                ph2_output = []
                result = futures[0].result()
                idx = 0
                for j in range(self._pixelhop_depth):
                    result_j_shrinked = shrink_args[j]['func'](result[j], shrink_args[j])
                    shape = np.array(np.shape(result_j_shrinked))
                    n_data_per_batch = shape[0]
                    shape[0] *= n_batch
                    ph2_output.append(np.empty(shape, dtype = np.float64))
                    ph2_output[j][idx : idx + n_data_per_batch] = result_j_shrinked
                idx += n_data_per_batch
                del result
                self._logger.debug('PixelHop Batch 0 Transform Done')
                continue
            result = futures[i].result()
            for j in range(self._pixelhop_depth):
                ph2_output[j][idx : idx + n_data_per_batch] = shrink_args[j]['func'](result[j], shrink_args[j])
            idx += n_data_per_batch
            del result
            self._logger.debug('PixelHop Batch (%d / %d) Transform Done' % (i, n_batch))
        futures.clear()
        self._logger.info('PixelHop Transform Done')
        for i in range(self._pixelhop_depth):
            self._logger.debug('Shape of PH Unit %d Output: %s = %d' % (i, np.shape(ph2_output[i]), np.size(ph2_output[i])))

        ph2_output_flatten = []
        for i in range(self._pixelhop_depth):
            ph2_output_flatten.append(np.reshape(ph2_output[i], (len(ph2_output[i]), -1)))
        del ph2_output

        if train:
            y_train = data_batch[0]['label']
            for i in range(1, n_batch):
                y_train = np.append(y_train, data_batch[i]['label'])

            ce = pixelhop.Cross_Entropy(num_class = self._data_n_class, num_bin = self._ce_n_centroids)
            ce_selected_idx = []
            for i in range(self._pixelhop_depth):
                ce_output = []
                n_features = np.shape(ph2_output_flatten[i])[-1]
                for j in range(n_features):
                    ce_output.append([])
                    x_ce_train = np.squeeze(ph2_output_flatten[i][:, j])
                    future = self._executor.submit(_bin_process_loss, ce, \
                        np.expand_dims(x_ce_train, axis = 1), y_train)
                    futures.append(future)
                for j in range(n_features):
                    ce_output[j] = futures[j].result()
                    self._logger.debug('Cross Entropy Unit %d Batch (%d / %d) Done' % (i, j, n_features))
                ce_selected_idx.append(np.argsort(ce_output)[:self._ce_n_features])
                del ce_output
                self._logger.info('Cross Entropy Unit %d Done' % i)
                futures.clear()
            self._ce_selected_idx = ce_selected_idx

        lags_output = []
        for i in range(self._pixelhop_depth):
            x_lag = ph2_output_flatten[i][:, self._ce_selected_idx[i]]
            if train:
                self._lags[i].fit(x_lag, y_train)
            lags_output.append(self._lags[i].transform(x_lag))
            self._logger.info('LAG Unit %d Done' % i)
        del ph2_output_flatten

        for i in range(self._pixelhop_depth):
            self._logger.debug('Shape of LAG Unit %d Output: %s' % (i, np.shape(lags_output[i])))

        lags_output_concat = np.reshape(np.swapaxes(lags_output, 0, 1), (len(lags_output[0]), -1))
        if train:
            self._final_classifier.fit(lags_output_concat, y_train)
            self._train = True
        y_predict = self._final_classifier.predict(lags_output_concat)
        self._logger.info('Final Classifier Done')
        if train:
            error_rate = np.count_nonzero(y_predict - y_train) / len(y_train)
            self._logger.info('Error Rate on Training Dataset: %.6f' % error_rate)
        return y_predict

    def _batch_dataset(self, x, y, n_batch, train):
        assert(n_batch > 1), 'Number of batches has to be more than one'
        if not train:
            data_split = np.split(x, n_batch)
            data_batch = []
            for i in range(len(data_split)):
                data_batch.append({'feature': np.array(data_split[i], dtype = np.float32), 'label': None})
            return data_batch

        assert(np.shape(x)[0] == np.shape(y)[0]), 'Size not match'
        labels = np.unique(y)
        _dict = {}
        for label in labels:
            _dict[label] = []
        for i in range(np.shape(x)[0]):
            _dict[y[i]].append(x[i])
        data_batch = []
        for i in range(n_batch):
            data_batch.append({'feature': [], 'label': []})
        for label in _dict.keys():
            for i in range(len(_dict[label])):
                data_batch[i % n_batch]['feature'].append(_dict[label][i])
                data_batch[i % n_batch]['label'].append(label)
        _dict.clear()
        for i in range(len(data_batch)):
            data_batch[i]['feature'] = np.array(data_batch[i]['feature'], dtype = np.float32)
            data_batch[i]['label'] = np.array(data_batch[i]['label'], dtype = np.uint8)
        return data_batch

    def _shrink(self, x, shrink_args):
        pool_win = shrink_args['pool_win']
        pool_stride = shrink_args['pool_stride']
        x_pooled = x
        if pool_win > 1 and pool_stride > 0:
            x_pooled = np.amax(np.squeeze(skimage.util.view_as_windows(x_pooled, \
                (1, pool_win, pool_win, 1), (1, pool_stride, pool_stride, 1)), \
                axis = (-4, -1)), axis = (-2, -1))

        neighborhood_win = shrink_args['neighborhood_win']
        neighborhood_stride = shrink_args['neighborhood_stride']
        if neighborhood_win < 2 or neighborhood_stride < 1:
            return x_pooled

        x_constructed = np.squeeze(skimage.util.view_as_windows(x_pooled, \
                (1, neighborhood_win, neighborhood_win, 1), (1, neighborhood_stride, neighborhood_stride, 1)), \
                axis = (-4, -1))
        shape = np.shape(x_constructed)
        return np.reshape(x_constructed, (shape[0], shape[1], shape[2], -1))
