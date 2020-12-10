#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include "JPEGEncodingGraph.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/Matrix.h"
#include "common/Utils.h"
#include "graph/Node.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ee669::ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("--quality", "50");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i").front();
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o").front();
    const auto quality = std::stoi(arguments.at("--quality").front());

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

    const auto inputKey = "input", decKey = "dec", encKey = "enc";
    auto graph = new ee669::JPEGEncodingGraph(inputKey, decKey, encKey, quality);
    auto resultNode = graph->createGraph();
    auto sess = (new ee669::Session())->feed(inputKey, &bytes);
    const auto encBytes = sess->run(resultNode);
    const auto oriBytes = sess->template get<ee669::Matrix<uint8_t>>(decKey)->getData();
    const auto codes = sess->template get<std::vector<uint8_t>>(encKey);
    delete sess;
    graph->deleteGraph();
    delete graph;

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
    const auto outputBitrate = static_cast<double>(CHAR_BIT * codes->size()) / oriBytes.size();
    const auto outputPSNR = ee669::Utils::calcPSNR(encBytes->cbegin(), encBytes->cend(),
        oriBytes.cbegin(), oriBytes.cend());
    const auto compressionRatio = static_cast<double>(inputSize) / outputSize;
    std::clog << "SUMMARY:\n"
              << "Executable: " << argv[0] << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Input Size: " << inputSize << " bits\n"
              << "Quality: " << quality << '\n'
              << "Output File: " << outputFilename << '\n'
              << "Output Size: " << outputSize << " bits\n"
              << "Output Compression Ratio: " << compressionRatio << '\n'
              << "Output Bitrate: " << outputBitrate << " bpp\n"
              << "Output PSNR: " << outputPSNR << " dB\n\n";

    return EXIT_SUCCESS;
}
