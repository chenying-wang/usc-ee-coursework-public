# EE-669 Homework #4

## Build
Build with gcc-8 and make

    $ make

## Usage

    $ ./bin/dct [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output DCT filename, if not specify then standard output used
- --width \<width\>
Width of input image, required
- --height \<height\>
Height of input image, required
- --block-size \<block_width\>
Size of blocks, '8' by default
- --quality \<quality_factor\>
Quality factor in quantization, '50' by default

[//]:#

    $ ./bin/jpeg [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output encoded filename, if not specify then standard output used
- --quality \<quality_factor\>
Quality factor in JPEG compression, '50' by default

[//]:#

    $ ./bin/postprocess [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output processed filename, if not specify then standard output used
- --ori \<original_file\>
Original image file used in PSNR calculation, not required
- --algo \<algorithm\>
Specify post-processing algorithm from 'lowpass' and 'reapply', required
- --boundary \<boundary_type\>
Specify boundary type used in shift in JPEG reapplication algorithm from 'symmetric', 'replication' or 'zero', 'zero' by default
- --quality \<quality_factor\>
Quality factor in JPEG reapplication algorithm, '50' by default

## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)
