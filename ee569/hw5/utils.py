# Name: Chenying Wang
# Email: chenying.wang@usc.edu
# USC ID: ****-****-**
# Date: Friday, April 3, 2020

import numpy as np

def nhwc2nchw(tensor):
    return np.transpose(tensor, [0, 3, 1, 2])

def nchw2nhwc(tensor):
    return np.transpose(tensor, [0, 2, 3, 1])
