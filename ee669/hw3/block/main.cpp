#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include "ScanOrderProcess.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/Matrix.h"
#include "common/StreamProcess.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ee669::ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("--width")
        ->addArgument("--height")
        ->addArgument("--block-width")
        ->addArgument("--block-height")
        ->addBoolArgument("--reverse");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i").front();
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o").front();
    const auto width = std::stoul(arguments.at("--width").front());
    const auto height = std::stoul(arguments.at("--height").front());
    const auto blockWidth = std::stoul(arguments.at("--block-width").front());
    const auto blockHeight = std::stoul(arguments.at("--block-height").front());
    const bool reverse = arguments.count("--reverse");

    ee669::BinaryFileIO binaryFileIO;
    std::ifstream fin;
    if (!isStdIn) {
        fin = binaryFileIO.openInput(inputFilename, std::ios::binary);
        if (fin.fail()) {
            std::cerr << "Failed to open input file " << inputFilename << '\n';
            return EXIT_FAILURE;
        }
    }
    auto bytes = binaryFileIO.read(isStdIn ? std::cin : fin);
    if (!isStdIn) {
        fin.close();
    }

    ee669::Matrix<uint8_t>::iterator *it = new ee669::Matrix<uint8_t>::blockingIterator(blockWidth, blockHeight);
    auto streamProcessChain = (new ee669::StreamProcessChain<uint8_t>())
        ->addLast(new ee669::ScanOrderProcess<uint8_t>(it, width, height, reverse));
    const auto codes = streamProcessChain->process(bytes);
    delete streamProcessChain;

    std::ofstream fout;
    if (!isStdOut) {
        fout = binaryFileIO.openOutput(outputFilename);
        if (fout.fail()) {
            std::cerr << "Failed to open output file " << outputFilename << '\n';
            return EXIT_FAILURE;
        }
    }
    binaryFileIO.write(isStdOut ? std::cout : fout, codes);
    if (!isStdOut) {
        fout.close();
    }

    const uint64_t inputSize = CHAR_BIT * bytes.size();
    std::clog << "SUMMARY:\n"
              << "Executable: " << argv[0] << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Input Size: " << inputSize << " bits\n"
              << "Output File: " << outputFilename << "\n\n";

    return EXIT_SUCCESS;
}
