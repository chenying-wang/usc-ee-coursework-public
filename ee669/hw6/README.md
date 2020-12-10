# EE-669 Homework #6

## Build
Build with gcc-8 and make

    $ make

## Usage

    $ ./bin/stat [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- --ori \<output_file\>
Reference filename, required
- -o \<output_file\>
Output absolute error map filename, if not specify then standard output used

[//]:#

In MATLAB,

    >> addpath('./src/app/ssim')
    >> ssim_main


## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)
