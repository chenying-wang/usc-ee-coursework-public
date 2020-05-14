/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __CSV_READER
#define __CSV_READER

#include <iostream>
#include <string>
#include <vector>

#define CSV_DELIMETER (',')
#define CSV_COMMENT ('#')

class CSVReader {
public:
    CSVReader();
    virtual ~CSVReader();

    std::vector<std::vector<std::string>> read(const char *csvFilename, const uint maxLength = 255) const;
};

#endif
