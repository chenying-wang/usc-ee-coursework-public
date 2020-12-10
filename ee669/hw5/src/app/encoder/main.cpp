#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <climits>

#include "EncoderNode.h"
#include "common/ArgParser.h"
#include "common/BinaryFileIO.h"
#include "common/Matrix.h"
#include "common/Utils.h"
#include "graph/Node.h"

int main(int argc, char *argv[]) {
    auto argParser = (new ee669::ArgParser())
        ->addArgument("-i")
        ->addArgument("-o")
        ->addArgument("--width")
        ->addArgument("--height")
        ->addArgument("--me")
        ->addBoolArgument("--no-emv")
        ->addBoolArgument("--no-etra");
    const auto arguments = argParser->parse(argc - 1, argc > 1 ? argv + 1 : nullptr);
    delete argParser;

    const bool isStdIn = !arguments.count("-i"), isStdOut = !arguments.count("-o");
    const auto inputFilename = isStdIn ? "stdin" : arguments.at("-i").front();
    const auto outputFilename = isStdOut ? "stdout" : arguments.at("-o").front();
    const auto width = std::stoul(arguments.at("--width").front());
    const auto height = std::stoul(arguments.at("--height").front());
    const auto me = arguments.count("--me") ? arguments.at("--me").front() : "";
    const bool noEMV = arguments.count("--no-emv");
    const bool noETRA = arguments.count("--no-etra");

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

    const auto inputKey = "input", psnrKey = "psnr", timeKey = "time";
    auto encoder = new ee669::EncoderNode("enc", inputKey, psnrKey, timeKey, width, height, me, noEMV, noETRA);
    auto sess = (new ee669::Session())->feed(inputKey, &bytes);
    const auto codes = sess->run(encoder);
    const auto psnr = sess->get<double>(psnrKey);
    const auto time = sess->get<double>(timeKey);
    delete sess;
    delete encoder;

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
              << "Output PSNR: " << *psnr << " dB\n"
              << "Output Motion Estimation Time: " << *time << " s\n\n";

    return EXIT_SUCCESS;
}
