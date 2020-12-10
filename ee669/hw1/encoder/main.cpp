#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include "Code.h"
#include "AdaptiveHuffmanEncoder.h"
#include "HuffmanEncoder.h"
#include "ShannonFanoEncoder.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/Utils.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ArgParser())
        ->addArgument("-a")
        ->addArgument("-i")
        ->addArgument("-o");
    const auto result = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    if (!result.count("-a")) {
        std::cerr << "Please specify codding algorithm\n";
        return EXIT_FAILURE;
    }

    const auto algorithm = result.at("-a");
    const bool isStdIn = !result.count("-i"), isStdOut = !result.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : result.at("-i");
    const auto outputFilename = isStdOut ? "stdout" : result.at("-o");

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

    auto stat = Utils::stat<std::byte>(bytes.cbegin(), bytes.cend());
    Encoder<std::byte> *encoder = nullptr;
    if (algorithm == "SF") {
        encoder = new ShannonFanoEncoder<std::byte>();
    } else if (algorithm == "Huffman") {
        encoder = new HuffmanEncoder<std::byte>();
    } else if (algorithm == "AdaHuffman") {
        encoder = new AdaptiveHuffmanEncoder<std::byte>();
    } else {
        std::cerr << "Unknown coding algorithm: " << algorithm << '\n';
        return EXIT_FAILURE;
    }

#ifndef __DEBUG
    const auto [codes, compressedSize] = encoder->build(stat)->encode(bytes);
#else
    const auto codebook = encoder->build(stat)->getCodebook();
    const auto [codes, compressedSize] = encoder->encode(bytes);
    for (const auto &e : codebook) {
        std::clog << (uint64_t) e.first << ':' << stat.at(e.first) << ':';
        for (auto i = e.second.codeLen - 1; i < e.second.codeLen; --i) {
            std::clog << e.second.code[i];
        }
        std::clog << '\n';
    }
#endif

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
    const double entropy = encoder->getEntropy(), avgCodeLen = (double) compressedSize / bytes.size();
    std::clog << "SUMMARY:\n"
              << "Coding Algorithm: " << algorithm << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Output File: " << outputFilename << '\n'
              << "Original Size: " << originalSize << " bits\n"
              << "Compressed Size: " << compressedSize << " bits\n"
              << "Compression Ratio: " << compressionRatio << "%\n"
              << "Input Entropy: " << entropy << " bits/symbol\n"
              << "Output Average Code Length: " << avgCodeLen << " bits/symbol\n"
              << "Coding Redundancy: " << avgCodeLen - entropy << " bits/symbol\n\n";
    delete encoder;

    return EXIT_SUCCESS;
}
