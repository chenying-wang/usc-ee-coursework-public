#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstddef>
#include <climits>

#include "ScalarQuantizer.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/StreamProcess.h"
#include "common/Utils.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ee669::ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("-b", "8")
        ->addArgument("--train");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i").front();
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o").front();
    const auto quantizationBits = std::stoi(arguments.at("-b").front());
    const auto trainingFiles = arguments.at("--train");

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

    std::vector<uint8_t> trainingBytes;
    for (const auto &trainingFile : trainingFiles) {
        fin = binaryFileIO.openInput(trainingFile, std::ios::binary);
        if (fin.fail()) {
            std::cerr << "Failed to open training file " << trainingFile << '\n';
            continue;
        }
        auto trBytes = binaryFileIO.read(fin);
        fin.close();
        trainingBytes.insert(trainingBytes.end(), trBytes.cbegin(), trBytes.cend());
        trBytes.clear();
    }

    const auto stat = ee669::Utils::stat(trainingBytes.cbegin(), trainingBytes.cend());

    auto quantizer = new ee669::ScalarQuantizer<uint8_t>(1 << quantizationBits, 0xff, 0x00, 1e-7, 100);
    auto streamProcessChain = (new ee669::StreamProcessChain<uint8_t>())
        ->addLast(quantizer->build(stat));
    const auto codes = streamProcessChain->process(bytes);
    delete streamProcessChain;

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

    const uint64_t inputSize = CHAR_BIT * bytes.size();
    const auto outputEntropy = ee669::Utils::calcEntropy(ee669::Utils::stat(codes.cbegin(), codes.cend()));
    const auto PSNR = ee669::Utils::calcPSNR(codes.cbegin(), codes.cend(), bytes.cbegin(), bytes.cend());
    std::ostringstream oss;
    std::copy(trainingFiles.begin(), trainingFiles.end() - 1, std::ostream_iterator<std::string>(oss, ", "));
    oss << trainingFiles.back();
    std::clog << "SUMMARY:\n"
              << "Executable: " << argv[0] << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Input Size: " << inputSize << " bits\n"
              << "Train Files: " << oss.str() << '\n'
              << "Quantization Bits: " << quantizationBits << '\n'
              << "Output File: " << outputFilename << '\n'
              << "Output Entropy: " << outputEntropy << " bits/symbol\n"
              << "Output PSNR: " << PSNR << "\n\n";

    return EXIT_SUCCESS;
}
