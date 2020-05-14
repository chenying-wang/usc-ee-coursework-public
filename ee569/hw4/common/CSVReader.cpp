/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#include <iostream>
#include <fstream>
#include <cstring>

#include "CSVReader.h"

CSVReader::CSVReader() {}

CSVReader::~CSVReader() {}

std::vector<std::vector<std::string>> CSVReader::read(const char *csvFilename, const uint maxLength) const {
    std::vector<std::vector<std::string>> ppphResult;
    std::ifstream fInput;
    fInput.open(csvFilename, std::ios::in);
    if (fInput.fail()) {
        std::cerr << "Error: Failed to read " << csvFilename << '\n';
        fInput.close();
        return ppphResult;
    }

    auto uiLine = 0u;
    while (!fInput.eof()) {
        auto phLine = new char[maxLength + 1];
        fInput.getline(phLine, maxLength);
        if (fInput.fail()) {
            break;
        }

        auto len = std::strlen(phLine);
        if (!len || phLine[0] == CSV_COMMENT) {
            delete[] phLine;
            continue;
        }

        std::vector<std::string> pphFields;
        for (auto i = 0u, j = 0u; i < len; ++j) {
            if (phLine[j] != CSV_DELIMETER && phLine[j] != '\0') {
                continue;
            }
            std::string field;
            field.assign(phLine + i, j - i);
            pphFields.push_back(field);
            i = j + 1;
        }
        delete[] phLine;
        ppphResult.push_back(pphFields);
        ++uiLine;
    }
    fInput.close();
#ifdef __VERBOSE
    std::cout << "Total " << uiLine << " lines read from " << csvFilename << '\n';
#endif
    return ppphResult;
}
