# EE-569 Homework #1

Name: Chenying Wang \<chenying.wang@usc.edu\> \
USC ID: ****-****-** \
Date: Friday, January 24, 2020


## Build

Both problems are supposed to be built simultaneously with GNU Make and compiled with GNU GCC-7.4.0 on Ubuntu 18.04 LTS. GCC has to support C++11 [ISO/IEC 14882:2011]

    $ make




## Usage

By default, please place the all input raw images in image_path (by default './img'), then run by

    $ make run <image_path>

Or in detail,

### 1. Image Demosaicing

    $ ./bin/demosaic [OPTION]... -i <input_file>

Options:
- -a \<algorithm\> \
Image demosaicing algorithm to use, 'bilinear' or 'mhc', by default 'mhc'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '600'
- -h \<height\> \
Height of input image, by default '532'
- --ori \<original_image\> \
Set original image as original_image to calculate PSNR, not mandatory


### 2. Image Manipulation

    $ ./bin/histogram [OPTION]... -i <input_file>

Options:
- -a \<algorithm\> \
Image histogram equalization to use, 'transfer_function' or 'bucket_filling', by default 'bucket_filling'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '400'
- -h \<height\> \
Height of input image, by default '560'
-c \<channel\>
Number of channles of input image, by default '3'
- --ori-pdf-output \<ori_pdf_file\> \
Output the histogram of input image to ori_pdf_file in csv format, not mandatory
- --transfer-function-output \<transfer_func_file\> \
Output the transfer function to transfer_func_file in csv format, for transfer_function only, not mandatory
- --cdf-output \<cdf_file\>
Output the cumulative histogram of processed image to cdf_file in csv format, not mandatory


### 3. Image Denoising

    $ ./bin/demosaic [OPTION]... -i <input_file>

Options:
- -a \<algorithm\> \
Image denoising algorithm to use, \
'linear_uniform', 'linear_gaussian', or 'bilateral', 'nlm', by default 'bilateral'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '320'
- -h \<height\> \
Height of input image, by default '320'
- --ori \<original_image\> \
Set original image as original_image to calculate PSNR, not mandatory
- --filter-width \<filter_width\> \
Width of filter to denoise, by default '7'
- --filter-height \<filter_height\> \
Height of filter to denoise, by default '7'
- --sigma-s \<sigma_s\> \
Set sigma of spatial distance, valid only if algorithm='gaussian' algorithm='bilateral', by default '0.90'
- --sigma-c \<sigma_c\> \
Set sigma of color intensity difference, valid only if algorithm='bilateral', by default '65'
- --neighborhood-width \<neighborhood_width\> \
Width of neighborhood window width, valid only if algorithm='nlm', by default '3'
- --neighborhood-height \<neighborhood_height\> \
Height of neighborhood window width, valid only if algorithm='nlm', by default '3'
- --nlm-h \<h\> \
Parameter h in NLM algorithm, valid only if algorithm='nlm', by default '20'
- --nlm-a \<a\> \
Parameter a in NLM algorithm, valid only if algorithm='nlm', by default '2.2'
- --analyze-noise-histogram \<noise_pdf_file\> \
Print pdf of noise to noise_pdf_file in csv format, dependent on original_image, not mandatory
- --analyze-diff-histogram \<diff_pdf_file\> \
Print pdf of noisy and original image, where intensity are not identical, to noise_pdf_file in csv format, dependent on original_image, not mandatory


### 4. BM3D
See [corn.m](./bm3d/corn.m)




## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)

### [MATLAB wrapper for BM3D denoising](./lib/tut_bm3d)
Copyright TAU \
Licensed under [LICENSE](./lib/tut_bm3d/LICENSE) \
<http://www.cs.tut.fi/~foi/GCF-BM3D/>
