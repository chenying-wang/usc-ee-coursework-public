#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include "LowpassProcessingGraph.h"
#include "ReapplyingJPEGGraph.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/Matrix.h"
#include "common/Utils.h"
#include "graph/Node.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ee669::ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("--ori")
        ->addArgument("--algo")
        ->addArgument("--boundary", "zero")
        ->addArgument("--quality", "50");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i").front();
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o").front();
    const auto oriFilename = arguments.count("--ori") ? arguments.at("--ori").front() : "";
    const auto algorithm = arguments.count("--algo") ? arguments.at("--algo").front() : "";
    const auto boundary = arguments.at("--boundary").front();
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

    ee669::Matrix<uint8_t>::Boundary bEnum;
    if (boundary == "symmetric") {
        bEnum = ee669::Matrix<uint8_t>::Boundary::ODD_SYMMETERIC;
    } else if (boundary == "replication") {
        bEnum = ee669::Matrix<uint8_t>::Boundary::REPLICATION;
    } else if (boundary == "zero") {
        bEnum = ee669::Matrix<uint8_t>::Boundary::ZERO_SHIFT;
    } else {
        std::cerr << "Unknown boundary: " << boundary << '\n';
        return EXIT_FAILURE;
    }

    ee669::Graph<std::vector<uint8_t>> *graph = nullptr;
    const auto inputKey = "input", decKey = "dec";
    if (algorithm == "lowpass") {
        graph = new ee669::LowpassProcessingGraph(inputKey, decKey, bEnum);
    } else if (algorithm == "reapply") {
        graph = new ee669::ReapplyingJPEGGraph(inputKey, decKey, bEnum, quality);
    } else {
        std::cerr << "Unknown algorithm: " << algorithm << '\n';
        return EXIT_FAILURE;
    }

    auto resultNode = graph->createGraph();
    auto sess = (new ee669::Session())->feed(inputKey, &bytes);
    const auto codes = sess->run(resultNode);
    const auto decBytes = sess->template get<ee669::Matrix<uint8_t>>(decKey)->getData();
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
    const auto inputPSNR = ee669::Utils::calcPSNR(decBytes.cbegin(), decBytes.cend(),
        oriBytes.cbegin(), oriBytes.cend());
    const auto outputPSNR = ee669::Utils::calcPSNR(codes->cbegin(), codes->cend(),
        oriBytes.cbegin(), oriBytes.cend());
    std::clog << "SUMMARY:\n"
              << "Executable: " << argv[0] << '\n'
              << "Input File: " << inputFilename << '\n'
              << "Input Size: " << inputSize << " bits\n"
              << "Input PSNR: " << inputPSNR << " dB\n"
              << "Algorithm: " << algorithm << '\n'
              << "Output File: " << outputFilename << '\n'
              << "Output Size: " << outputSize << " bits\n"
              << "Output PSNR: " << outputPSNR << " dB\n\n";

    return EXIT_SUCCESS;
}
