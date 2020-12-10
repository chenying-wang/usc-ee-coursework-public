#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include "DCTNode.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "graph/Node.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ee669::ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("--width")
        ->addArgument("--height")
        ->addArgument("--block-size", "8")
        ->addArgument("--quality", "50");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i").front();
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o").front();
    if (!(arguments.count("--width") && arguments.count("--height"))) {
        std::cerr << "Please specify width and height of input image\n";
        return EXIT_FAILURE;
    }
    const auto width = std::stoul(arguments.at("--width").front());
    const auto height = std::stoul(arguments.at("--height").front());
    const auto blockSize = std::stoul(arguments.at("--block-size").front());
    const auto quality = std::stoul(arguments.at("--quality").front());

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

    const auto inputKey = "input";
    auto dct = new ee669::DCTNode("dct", inputKey, width, height, blockSize, quality);
    auto sess = (new ee669::Session())->feed(inputKey, &bytes);
    const auto codes = sess->run(dct);
    delete sess;
    delete dct;

    std::ofstream fout;
    if (!isStdOut) {
        fout = binaryFileIO.openOutput(outputFilename);
        if (fout.fail()) {
            std::cerr << "Failed to open output file " << outputFilename << '\n';
            return EXIT_FAILURE;
        }
    }
    binaryFileIO.write(isStdOut ? std::cout : fout, *codes);
    if (!isStdOut) {
        fout.close();
    }

    const uint64_t inputSize = CHAR_BIT * bytes.size(), outputSize = CHAR_BIT * codes->size();
    std::clog << "SUMMARY:\n"
              << "Executable: " << argv[0] << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Input Size: " << inputSize << " bits\n"
              << "Input Width: " << width << '\n'
              << "Input Height: " << height << '\n'
              << "Block Size: " << blockSize << '\n'
              << "Quality: " << quality << '\n'
              << "Output File: " << outputFilename << '\n'
              << "Output Size: " << outputSize << " bits\n\n";

    return EXIT_SUCCESS;
}
