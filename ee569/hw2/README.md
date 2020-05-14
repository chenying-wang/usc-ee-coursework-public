# EE-569 Homework #2

Name: Chenying Wang \<chenying.wang@usc.edu\> \
USC ID: ****-****-** \
Date: Friday, February 14, 2020


## Build

Both problems are supposed to be built simultaneously with GNU Make and compiled with GNU GCC-7.4.0 on Ubuntu 18.04 LTS. GCC has to support C++11 [ISO/IEC 14882:2011]

    $ make




## Usage

By default, please place the all input raw images in image_path (by default './img'), then run by

    $ make run <image_path>

Or in detail,

### 1. Edge Detection

    $ ./bin/edge [OPTION]... -i <input_file>

Options:
- -a \<algorithm\> \
Edge detector to use, 'sobel' or 'canny'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '481'
- -h \<height\> \
Height of input image, by default '321'
- -c \<channel\>
Number of channels of input image, by default '3'
- --threshold \<threshold\>
Threshold used in sobel and canny (as upper threshold), by default '128'
- --x-gradient-output  \<x_gradient_img\>
Output X gradient image to x_gradient_img, for sobel only, not mandatory
- --y-gradient-output \<y_gradient_img\>
Output Y gradient image to y_gradient_img, for sobel only, not mandatory
- --gradient-output \<gradient_img\>
Output gradient map to gradient_img, for sobel only, not mandatory
- --low-threshold \<low_threshold\>
Lower threshold used in canny, for canny only, by default '64'

### 2. Digital Half-toning

    $ ./bin/halftone [OPTION]... -i <input_file>

Options:
- -a \<algorithm\> \
Half-toning algorithm to use, 'sobel' or 'canny'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '750'
- -h \<height\> \
Height of input image, by default '500'
- -c \<channel\>
Number of channels of input image, by default '1'
- --threshold \<threshold\>
Threshold used in fixed, error_diffusion_fs, error_diffusion_jjn and error_diffusion_stucki, by default '128'
- --dithering-matrix-size \<dithering_matrix_size\>
To use 2^dithering_matrix_size dithering matrix, for dithering_matrix only, by default '1'

### 3. Structured Edge
See [demo.m](./structured_edge/demo.m)

To detect edge by structured edge

    >> detectEdge(oriImg, edgeImg, width, height)

To evaluate half-toned image with ground truth

    >> evalEdgeImg(edgeImg, groundTruth, width, height, thrs)




## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)

### [Piotr's Computer Vision Matlab Toolbox](./lib/pdollar_toolbox)
By Piotr Dollar \
<https://github.com/pdollar/toolbox>

### [Structured Edge Detection Toolbox V3.0](./lib/pdollar_edges)
Copyright (c) Microsoft Corporation \
Licensed under under the MSR-LA Full Rights License \
See [license.txt](./lib/pdollar_edges/license.txt) \
<https://github.com/pdollar/edges>
