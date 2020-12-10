#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include "StatGraph.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/Matrix.h"
#include "common/Utils.h"
#include "graph/Node.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ee669::ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("--ori");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i").front();
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o").front();
    const auto oriFilename = arguments.count("--ori") ? arguments.at("--ori").front() : "";

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

    std::vector<uint8_t> oriBytes;
    if (!oriFilename.empty()) {
        fin = binaryFileIO.openInput(oriFilename, std::ios::binary);
        if (!fin.fail()) {
            oriBytes = binaryFileIO.read(fin);
            fin.close();
        } else {
            std::cerr << "Failed to open input file " << inputFilename << '\n';
        }
    }

    const auto inputKey = "input", oriKey = "ori", mseKey = "mse", psnrKey = "psnr";
    auto graph = new ee669::StatGraph(inputKey, oriKey, mseKey, psnrKey);
    auto resultNode = graph->createGraph();
    auto sess = (new ee669::Session())
        ->feed(inputKey, &bytes)
        ->feed(oriKey, &oriBytes);
    const auto codes = sess->run(resultNode);
    const auto mse = *sess->template get<double>(mseKey);
    const auto psnr = *sess->template get<double>(psnrKey);
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
    std::clog << "SUMMARY:\n"
              << "Executable: " << argv[0] << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Input Size: " << inputSize << " bits\n"
              << "Output File: " << outputFilename << '\n'
              << "Output Size: " << outputSize << " bits\n"
              << "MSE: " << mse << '\n'
              << "PSNR: " << psnr << " dB\n\n";

    return EXIT_SUCCESS;
}
