#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstddef>

#include "BinaryFileIO.h"

BinaryFileIO::BinaryFileIO() {}

BinaryFileIO::~BinaryFileIO() {}

std::ifstream
BinaryFileIO::openInput(const std::string &filename, const std::ios::openmode openmode) const {
    const auto status = std::filesystem::status(filename);
    if (!std::filesystem::exists(status)) {
        throw std::runtime_error("File " + filename + " not exits");
    } else if (std::filesystem::is_directory(status)) {
        throw std::runtime_error("File " + filename + " is directory");
    }

    std::ifstream fin;
    fin.open(filename, std::ios::in | openmode);
    if (fin.fail()) {
        throw std::runtime_error("Failed to open file " + filename);
    }
    return fin;
}

std::ofstream
BinaryFileIO::openOutput(const std::string &filename, const std::ios::openmode openmode) const {
    const auto status = std::filesystem::status(filename);
    if (std::filesystem::is_directory(status)) {
        throw std::runtime_error("File " + filename + " is directory");
    }

    const auto parentPath = std::filesystem::path(filename).parent_path();
    const auto parentPathStatus = std::filesystem::status(parentPath);
    if (!std::filesystem::exists(parentPathStatus)) {
       std::filesystem::create_directories(parentPath);
    }

    std::ofstream fout;
    fout.open(filename, std::ios::out | openmode);
    if (fout.fail()) {
        throw std::runtime_error("Failed to open file " + filename);
    }
    return fout;
}

std::vector<std::byte> BinaryFileIO::read(std::istream &input) const {
    auto bytes = std::vector<std::byte>();
    const auto bufferSize = 4096;
    auto buffer = new std::byte[bufferSize];
    while (input.good()) {
        input.read(reinterpret_cast<char *>(buffer), bufferSize);
        bytes.insert(bytes.end(), buffer, buffer + input.gcount());
    }
    delete[] buffer;
    return bytes;
}

void BinaryFileIO::write(std::ostream &output, const std::vector<std::byte> &bytes) const {
    if (output.fail() || bytes.empty()) {
        return;
    }
    output.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}
