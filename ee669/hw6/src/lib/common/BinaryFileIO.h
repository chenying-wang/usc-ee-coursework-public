#ifndef __EE669_BINARY_FILE_IO
#define __EE669_BINARY_FILE_IO

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstddef>

namespace ee669 {
    class BinaryFileIO {
    private:
        const static uint64_t BUFFER_SIZE = 4096UL;

    public:
        BinaryFileIO();
        virtual ~BinaryFileIO();

        std::ifstream openInput(const std::string &filename, const std::ios::openmode openmode = std::ios::in) const;
        std::ofstream openOutput(const std::string &filename, const std::ios::openmode openmode = std::ios::out) const;
        std::vector<uint8_t> read(std::istream &input) const;
        void write(std::ostream &output, const std::vector<uint8_t> &bytes) const;
    };
}

#endif
