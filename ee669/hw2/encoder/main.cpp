#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include <bitset>

#include "BitPlaneMappingProcess.h"
#include "Code.h"
#include "Encoder.h"
#include "QMEncoder.h"
#include "ScanOrderProcess.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/Utils.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("--preprocessing", "")
        ->addBoolArgument("--bit-plane")
        ->addArgument("--context-bits", "0")
        ->addArgument("--width", "0")
        ->addArgument("--height", "0");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i");
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o");
    const auto preprocessing = arguments.at("--preprocessing");
    const bool bitplaneMapping = arguments.count("--bit-plane");
    const auto numContextBits = std::stoul(arguments.at("--context-bits"));
    const auto width = std::stoul(arguments.at("--width"));
    const auto height = std::stoul(arguments.at("--height"));

    BinaryFileIO binaryFileIO;
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

    Matrix<std::byte>::iterator *it = nullptr;
    if (preprocessing == "zigzag") {
        it = new Matrix<std::byte>::zigzagIterator();
    } else if (preprocessing == "zorder" && width > 0 && height > 0) {
        it = new Matrix<std::byte>::zOrderIterator();
    } else if (preprocessing == "hilbert" && width > 0 && height > 0) {
        it = new Matrix<std::byte>::hilbertIterator();
    } else {
        it = new Matrix<std::byte>::iterator();
    }
    auto byteStreamProcessChain = (new ByteStreamProcessChain<std::byte>())
        ->addLast(new ScanOrderProcess<std::byte>(it, width, height))
        ->addLast(new BitPlaneMappingProcess<std::byte>(bitplaneMapping))
        ->addLast(new QMEncoder<std::byte>(new BitsContextModel(numContextBits)));

    const auto codes = byteStreamProcessChain->process(bytes);
    const auto compressedSize = CHAR_BIT * codes.size();

    std::ofstream fout;
    if (!isStdOut) {
        fout = binaryFileIO.openOutput(outputFilename, std::ios::binary);
        if (fout.fail()) {
            std::cerr << "Failed to open output file " << outputFilename << '\n';
            return EXIT_FAILURE;
        }
    }
    binaryFileIO.write(isStdOut ? std::cout : fout, codes);
    if (!isStdOut) {
        fout.close();
    }

    const uint64_t originalSize = CHAR_BIT * bytes.size();
    const double compressionRatio = 100.0 - 100.0 * compressedSize / originalSize;
    std::clog << "SUMMARY:\n"
              << "Number of Context: " << numContextBits << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Output File: " << outputFilename << '\n'
              << "Original Size: " << originalSize << " bits\n"
              << "Compressed Size: " << compressedSize << " bits\n"
              << "Compression Ratio: " << compressionRatio << "%\n\n";

    return EXIT_SUCCESS;
}
