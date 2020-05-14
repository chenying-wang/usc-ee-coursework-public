import numpy as np
import matplotlib.pyplot as plt
import sys

def get_line(weight, linespace):
    return np.array(-(weight[1] * linespace + weight[0]) / weight[2])

def plot_area(ax, weights_group, linespace, label = None, **kwargs):
    areas = []
    for weights in weights_group:
        lower_bound = np.full(np.size(linespace), -sys.maxsize)
        upper_bound = np.full(np.size(linespace), sys.maxsize)
        lower_vbound = -sys.maxsize
        upper_vbound = sys.maxsize
        for weight in weights:

            if (weight[2] > 0):
                line = get_line(weight, linespace)
                lower_bound = np.maximum(line, lower_bound)
            elif (weight[2] < 0):
                line = get_line(weight, linespace)
                upper_bound = np.minimum(line, upper_bound)
            else:
                x = -weight[0] / weight[1]
                if (weight[1] > 0):
                    lower_vbound = np.maximum(x, lower_vbound)
                elif (weight[1] < 0):
                    upper_vbound = np.minimum(x, upper_vbound)

        for weight in weights:
            if (weight[2] == 0):
                x = -weight[0] / weight[1]
                idx = np.argmin(np.abs(linespace - x))
                ax.vlines(x, \
                    ymin = lower_bound[idx], \
                    ymax = upper_bound[idx], \
                    color = 'black', alpha = 0.5)
            else:
                line = get_line(weight, linespace)
                where = np.all([\
                    upper_bound > lower_bound, \
                    linespace > lower_vbound, \
                    linespace < upper_vbound, \
                    line >= lower_bound, \
                    line <= upper_bound], axis = 0)
                ax.plot(linespace[where], line[where], \
                    color = 'black', alpha = 0.5)
        where = np.all([\
            upper_bound > lower_bound, \
            linespace > lower_vbound, \
            linespace < upper_vbound], axis = 0)
        ax.fill_between(linespace, upper_bound, lower_bound, \
                where = where, \
                label = label, **kwargs)
