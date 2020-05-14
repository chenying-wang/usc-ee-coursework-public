# EE-569 Homework #3

Name: Chenying Wang \<chenying.wang@usc.edu\> \
USC ID: ****-****-** \
Date: Friday, February 28, 2020


## Build

Both problems are supposed to be built simultaneously with GNU Make and compiled with GNU GCC-7.4.0 on Ubuntu 18.04 LTS. GCC has to support C++11 [ISO/IEC 14882:2011]

    $ make




## Usage

By default, please place the all input raw images in image_path (by default './img'), then run by

    $ make run <image_path>

Or in detail,

### 1. Geometric Warping

    $ ./bin/warp [OPTION]... -i <input_file>

Options:
- -a \<algorithm\> \
Warping approach to use, 'warp' or 'reverse', by default 'warp'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '512'
- -h \<height\> \
Height of input image, by default '512'
- -c \<channel\> \
Number of channels of input image, by default '3'

### 2. Image Stitching

    $ ./bin/stitch [OPTION]... --left <left_img> --middle <middle_img> --right <right_img>

Options:
- -a \<algorithm\> \
Image stitching approach to use, 'stitch' only, by default 'stitch'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '480'
- -h \<height\> \
Height of input image, by default '720'
- -c \<channel\> \
Number of channels of input image, by default '3'
- --lm-matches \<left_middle_matches_img\> \
Output all matches from SURF and FLANN between left and middle image
as raw image to left_middle_matches_img, not mandatory
- --mr-matches \<middle_right_matches_img\> \
Output all matches from SURF and FLANN between middle and right image
as raw image to middle_right_matches_img, not mandatory
- --lm-best-matches \<left_middle_best_matches_img\> \
Output four chosen matches from SURF and FLANN between left and middle image
as raw image to --lm-best-matches \<left_middle_best_matches_img\> \
, not mandatory
- --mr-best-matches \<middle_right_best_matches_img\> \
Output four chosen matches from SURF and FLANN between middle and right image
as raw image to middle_right_best_matches_img, not mandatory

### 3. Geometric Warping

    $ ./bin/morph [OPTION]... -i <input_file>

Options:
- -a \<algorithm\> \
Morphological processing operation/algorithm to use, 'shrink', 'thin', 'skeletonize'
, 'count', 'count_dfs', 'pcb_hole', 'pcb_pathway', 'defect', by default 'shrink'
- -o \<output_file\> \
Output raw image to output_file, by default './result.raw'
- -w \<width\> \
Width of input image, by default '558'
- -h \<height\> \
Height of input image, by default '558'
- -c \<channel\> \
Number of channels of input image, by default '1'
- --threshold \<threshold\> \
Image binarization threshold, by default '127'
- --stage1-masks \<stage1_masks_csv\> \
Conditional mark patterns CSV file for the morphological processing
- --stage2-masks \<stage2_masks_csv\> \
Unconditional mark patterns CSV file for the morphological processing
- --iteration \<iteration\> \
Maximum iterations allowed, for all except count_dfs, by default '1000'
- --count-csv \<count_csv\> \
Output frequency of size as CSV file to count_csv
, for count and count_dfs only, not mandatory
- --track \<track_video\> \
Output track of intermediate results
, for shrink, thin, skeletonize, pcb_pathway and defect only
, not mandatory

## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)
