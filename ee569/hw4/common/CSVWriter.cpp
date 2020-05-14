/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <fstream>

#include "CSVWriter.h"

CSVWriter::CSVWriter() {}

CSVWriter::~CSVWriter() {}

uint CSVWriter::write(const char *csvFilename, const std::vector<std::vector<std::string>> data) const {
    if (csvFilename == nullptr) {
        return 0u;
    }

    std::ofstream fOutput;
    fOutput.open(csvFilename, std::ios::out);
    if (fOutput.fail()) {
        std::cerr << "Error: Failed to read " << csvFilename << '\n';
        fOutput.close();
        return 0u;
    }

    auto count = 0u;
    for (auto line : data) {
        if (!line.size()) {
            continue;
        }
        auto size = line.size();
        fOutput << line.front();
        for (auto i = 1u; i < size; ++i) {
            fOutput << ',' << line.at(i);
        }
        fOutput << '\n';
        ++count;
    }
    fOutput.close();
    return count;
}
