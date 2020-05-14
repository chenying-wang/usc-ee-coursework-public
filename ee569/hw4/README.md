# EE-569 Homework #4

Name: Chenying Wang \<chenying.wang@usc.edu\> \
USC ID: ****-****-** \
Date: Friday, March 20, 2020


## Build

Both problems are supposed to be built simultaneously with GNU Make and compiled with GNU GCC-7.5.0 on Ubuntu 18.04 LTS. GCC has to support C++11 [ISO/IEC 14882:2011]

    $ make




## Usage

By default, please place the all input raw images in image_path (by default './img'), then run by

    $ make run <image_path>

Or in detail,

### 1. Texture Analysis and Segmentation

    $ ./bin/texture [OPTION]...

Options:
- -a \<algorithm\> \
Warping approach to use, 'pca', 'k_means', 'rf', 'svm' or 'k_means_segment', by default 'pca'
- -i <input_file> \
Input raw image for texture segmentation, for 'k_means_segment' only
- -o \<output_file\> \
Output segmented raw image to output_file, not mandatory
- -w \<width\> \
Width of input image for texture segmentation, for 'k_means_segment' only, by default '600'
- -h \<height\> \
Height of input image for texture segmentation, for 'k_means_segment' only, by default '450'
- -c \<channel\> \
Number of channels of input image, by default '1'
- --train \<training_dataset_csv\> \
See [train_dataset.csv](./config/train_dataset.csv), by default '600'
- --test \<training_dataset_csv\> \
See [test_dataset.csv](./config/test_dataset.csv), by default '600'
- --dataset-width \<dataset_width\> \
Width of dataset images, by default '128'
- --dataset-height \<dataset_height\> \
Height of dataset images, by default '128'
- --laws-kernel \<laws_kernel_csv\> \
Laws kernels CSV, by default '600'
- --pca <\pca\> \
PCA dimension
- --train-pca-csv \<train_pca_csv\> \
Training dataset pca feature CSV Output, for 'pca' only
- --test-pca-csv \<test_pca_csv\> \
Test dataset pca feature CSV Output, for 'pca' only
- --cluster \<cluster\> \
Number of cluster, for 'k_means_segment' only, by default '6'
- --window-size \<window_size\> \
Pooling window size, for 'k_means_segment' only, by default '15'
- --opt-windows \
If use optimized windows, for 'k_means_segment' only

### 2. Image Feature Extractors

    $ ./bin/feature [OPTION]...

Options:
- -a \<algorithm\> \
Warping approach to use, 'keypoints', 'largest_keypoints', 'match', 'match_largest_scale',
'bag_of_words' and 'match_bag_of_words', by default 'keypoints'
- --query \<input_image\> \
First input image
- --train \<input_image\> \
Second input image, for 'match', 'match_largest_scale' and 'match_bag_of_words' only
- -o \<output_file\> \
Output raw image to output_file, not mandatory
- --output-csv \<output_csv\> \
Histogram CSV output, for 'bag_of_words' and 'match_bag_of_words' only





## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)
