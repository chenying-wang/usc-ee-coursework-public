/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, March 20, 2020
 **/
#ifndef __CSV_WRITER
#define __CSV_WRITER

#include <iostream>
#include <vector>

class CSVWriter {
public:
    CSVWriter();
    ~CSVWriter();

    uint write(const char *csvFilename, const std::vector<std::vector<std::string>> data) const;
};

#endif
