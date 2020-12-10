# EE-669 Homework #5

## Build
Build with gcc-8 and make

    $ make

## Usage

    $ ./bin/encoder [OPTION]...

Options:
- -i \<input_file\>
Input source filename, if not specify then standard input used
- -o \<output_file\>
Output filename, if not specify then standard output used
- --width \<width\>
Width of input video sequence, required
- --height \<height\>
Height of input video sequence, required
- --me \<me_method\>
Specify motion estimation algorithm from 'dia', 'hex', 'umh' and 'esa'
- --no-emv
Specify no extra motion vector for ME initialization
- --no-etra
Specify no early termination and range adaptation algorithm in UMH search

[//]:#

## License

Copyright (c) 2020 Chenying Wang \<chenying.wang@usc.edu\> \
Licensed under MIT License \
See [LICENSE](./LICENSE)

### [x264](https://code.videolan.org/videolan/x264)

Copyright (C) 2003-2020 x264 project \
Licensed under GNU General Public License, version 2 \
See [COPYING](./src/lib/x264/COPYING)
