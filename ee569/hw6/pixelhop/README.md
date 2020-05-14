# EE569_2020Spring_HW6: Successive Subspace Learning
### Introduction
This is an implementation by Yifan Wang for the paper [PixelHop++: A Small Successive-Subspace-Learning-Based (SSL-based) Model for Image Classification](https://arxiv.org/abs/2002.03141). Note that this is not the official implementation. The functions are provided as the source code for EE569 Spring20 HW6. 
### Installation
You need to install numpy and sklearn packages to use the code.
### Contents
`saab.py`: Saab transform\
`cwSaab.py`: Channel-wise Saab transform. Use energy threshold `TH1` to choose intermediate nodes.\
`pixelhop2.py`: Prune the tree built by `cwSaab.py` (choose discarded nodes) using threshold `TH2`.\
`cross_entropy.py`: Compute cross entropy for a single feature dimension to do feature selection\
`lag.py`: Label-Assisted reGression (LAG unit)\
`llsr.py`: Assembled LLSR module ([numpy.linalg.lstsq](https://docs.scipy.org/doc/numpy/reference/generated/numpy.linalg.lstsq.html) is used inside)\
Example of usage can be found at the bottom of each file. \
**Note**: All the images or data that are fed into these functions should be in the `channel last` format.
