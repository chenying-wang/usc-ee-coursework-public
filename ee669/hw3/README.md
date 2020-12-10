# EE-669 Homework #3

## Build
Build with gcc-8 and make

    $ make

## Usage

    $ ./bin/histogram [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output histogram CSV filename, if not specify then standard output used

[//]:#

    $ ./bin/quantizer [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output encoded filename, if not specify then standard output used
- -b \<bits\>
Quantization bits, if not specify then 8 bits will be used
- --train \<train_file\>...
Training files, required

[//]:#

    $ ./bin/block [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output blocking/unblocking filename, if not specify then standard output used
- --width \<width\>
Width of input image, required
- --height \<height\>
Height of input image, required
- --block-width \<block_width\>
Width of blocks, required
- --block-height \<block_height\>
Height of blocks, required
- --reverse
If specified, unblocking is used

## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)
