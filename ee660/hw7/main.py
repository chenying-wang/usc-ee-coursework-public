#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

COLOR = ['#4285f4', '#db4437', '#f4b400', '#0f9d58', '#303030']

WEIGHTS = np.array([
    [10, 0], [10, 2], [10, 4], [10, 6], [8, 6],
    [8, 8], [6, 8], [6, 10], [4, 10], [2, 10]
], dtype = np.float)

def drawLine(ax, x1, y1, x2, y2, color = 'black', alpha = 1.0, label = None):
    ax.arrow(x1, y1, x2 - x1, y2 - y1, width = 0, linewidth = 1,
        color = color, alpha = alpha, label = label)

def drawCircle(ax, x, y, radius, color = 'black', alpha = 1.0, label = None):
    circle = plt.Circle((x, y), radius, linewidth = 1, fill = False,
        edgecolor = color, facecolor = 'white', alpha = alpha)
    ax.add_artist(circle)

def drawL1Region(ax, C):
    drawLine(ax, C, 0, 0, C, color = COLOR[4], alpha = 0.8)
    drawLine(ax, C, 0, 0, -C, color = COLOR[4], alpha = 0.8)
    drawLine(ax, -C, 0, 0, C, color = COLOR[4], alpha = 0.8)
    drawLine(ax, -C, 0, 0, -C, color = COLOR[4], alpha = 0.8)

def drawL2Region(ax, C):
    drawCircle(ax, 0, 0, C, color = COLOR[4], alpha = 0.8)

def drawSquareRegion(ax, C):
    drawLine(ax, C, -C, C, C, color = COLOR[4], alpha = 0.8)
    drawLine(ax, -C, C, C, C, color = COLOR[4], alpha = 0.8)
    drawLine(ax, -C, C, -C, -C, color = COLOR[4], alpha = 0.8)
    drawLine(ax, C, -C, -C, -C, color = COLOR[4], alpha = 0.8)

def l1Regularize(weights, C):
    reg = np.empty_like(weights, dtype = np.float)
    for i in range(len(weights)):
        x, y = weights[i, 0], weights[i, 1]
        if y - x <= -C:
            reg[i] = C, 0
            continue
        elif y - x >= C:
            reg[i] = 0, C
            continue
        d = (x + y - C) / 2
        reg[i] = x - d, y - d
    return reg

def l2Regularize(weights, C):
    norm = np.linalg.norm(weights, axis = 1)
    return C * weights / np.column_stack((norm, norm))

def lpRegularize(weights, C):
    return np.minimum(weights, C)

def calcSparsity(arr, axis):
    return np.size(arr, axis) - np.count_nonzero(arr, axis)

def main(weights, region, C, plot_fname, idx = []):
    fig, ax = plt.subplots()
    ax.set_aspect('equal')

    inv_idx = np.ones(len(weights))
    inv_idx[idx] = 0
    inv_idx = inv_idx == 1
    ax.scatter(weights[inv_idx, 0], weights[inv_idx, 1], color = COLOR[0], label = 'Unconstrained')
    ax.scatter(weights[idx, 0], weights[idx, 1], color = COLOR[2], label = 'Unconstrained')
    if region == 'L1':
        drawL1Region(ax, C)
        weights_reg = l1Regularize(weights, C)
    elif region == 'L2':
        drawL2Region(ax, C)
        weights_reg = l2Regularize(weights, C)
    else:
        drawSquareRegion(ax, C)
        weights_reg = lpRegularize(weights, C)

    sparsity = calcSparsity(weights, axis = 1)
    sparsity_reg = calcSparsity(weights_reg, axis = 1)
    print(f'Number of weights with higher regularization sparsity: {np.sum(sparsity_reg > sparsity)}')

    ax.scatter(weights_reg[:, 0], weights_reg[:, 1], color = COLOR[1], alpha = 0.5, label = 'Regularized')
    for i in idx:
        drawLine(ax, weights[i, 0], weights[i, 1], weights_reg[i, 0], weights_reg[i, 1],
            color = COLOR[2], alpha = 0.5)
        radius = np.linalg.norm(weights[i] - weights_reg[i])
        drawCircle(ax, weights[i, 0], weights[i, 1], radius,
            color = COLOR[2], alpha = 0.5)

    ax.set_xlim(-5.5, 10.5)
    ax.set_ylim(-5.5, 10.5)
    ax.grid(alpha = 0.3)
    ax.legend()
    fig.savefig(plot_fname, dpi = 600, transparent = True)
    plt.clf()

if __name__ == "__main__":
    main(WEIGHTS, 'L2', 2, './fig_a.pdf', [1, 6])
    main(WEIGHTS, 'L1', 2, './fig_b.pdf', [2, 5, 9])
    main(WEIGHTS, 'Lp', 1, './fig_c.pdf', [0, 5, 9])
    main(WEIGHTS, 'L2', 5, './fig_d.pdf', [1, 6])
    main(WEIGHTS, 'L1', 5, './fig_e.pdf', [0, 5, 8])
