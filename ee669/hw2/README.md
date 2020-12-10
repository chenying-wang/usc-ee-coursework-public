# EE-669 Homework #2

## Build
Build with gcc-8 and make

    $ make

## Usage

    $ ./bin/encode [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output encoded filename, if not specify then standard output used
- --preprocessing \<preprocess_method\>
Specify the preprocessing method from 'zigzag', 'zorder', 'hilbert' or leaving blank for no preprocessing
- --bit-plane
Specify whether use bit-plane mapping or not
- --context-bits \<num_context_bits\>
Specify the number of context bits in CABAC, '0' and by default for BAC
- --width \<img_width\>
Specify the width of image used in preprocessing
- --height \<img_height\>
Specify the height of image used in preprocessing

## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)
