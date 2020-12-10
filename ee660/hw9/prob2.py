#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

COLOR = ['#4285f4', '#db4437', '#f4b400', '#0f9d58', '#303030']

def gaussian(x, mean, var):
    return np.exp(-(x - mean)**2 / (2 * var**2)) / np.sqrt(2 * np.pi * var)

def stepE(x, pi, mean, var):
    alpha = 0
    for i in range(len(pi)):
        alpha += gaussian(x, mean[i], var[i]) * pi[i]
    gamma = []
    for i in range(len(pi)):
        gamma.append(gaussian(x, mean[i], var[i]) * pi[i] / alpha)
    return gamma

def stepM(x, gamma, data):
    mean = []
    for i in range(len(gamma)):
        mean.append((gamma[i] * x + np.sum(data[i])) / (gamma[i] + len(data[i])))
    return mean

def prob(x, data, pi, mean, var):
    prob = 0.0
    for i in range(len(pi)):
        prob += gaussian(x, mean[i], var[i])
    for i in range(len(pi)):
        for j in range(len(data[i])):
            prob *= gaussian(data[i][j], mean[i], var[i]) * pi[i]
    return prob

def main(x, data, pi, mean, var, gamma_plot, mean_plot):
    if len(data) != len(pi):
        return

    gammas, means = [], []
    means.append(mean)
    prev = 0.0
    for i in range(100):
        gamma = stepE(x, pi, mean, var)
        mean = stepM(x, gamma, data)
        gammas.append(gamma)
        means.append(mean)
        proba = prob(x, data, pi, mean, var)
        if i > 0 and (proba - prev) / prev < 1e-9:
            break
        prev = proba

    gammas, means = np.array(gammas), np.array(means)
    print('mu: %s' % means[-1])
    print('gamma: %s' % gammas[-1])

    fig, ax = plt.subplots()
    ax.plot(np.arange(len(gammas)), gammas[:, 0], color = COLOR[0], label = 'gamma_1')
    ax.plot(np.arange(len(gammas)), gammas[:, 1], color = COLOR[1], label = 'gamma_2')
    ax.set_xticks(1 + np.arange(len(gammas)))
    ax.grid(alpha = 0.3)
    ax.legend()
    fig.savefig(gamma_plot, dpi = 600, transparent = True)
    plt.clf()

    fig, ax = plt.subplots()
    ax.plot(np.arange(len(means)), means[:, 0], color = COLOR[0], label = 'mu_1')
    ax.plot(np.arange(len(means)), means[:, 1], color = COLOR[1], label = 'mu_2')
    ax.set_xticks(np.arange(len(means)))
    ax.grid(alpha = 0.3)
    ax.legend()
    fig.savefig(mean_plot, dpi = 600, transparent = True)
    plt.clf()

if __name__ == "__main__":
    data = [[1, 2], [4]]
    pi = [0.5, 0.5]
    mean = [1.5, 4.0]
    var = [1, 1]
    main(3, data, pi, mean, var, './gamma.pdf', './mean.pdf')
