/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, February 14, 2020
 **/
#ifndef __UTILS
#define __UTILS

class Utils {
private:
    Utils();
    virtual ~Utils();
public:
    static char *getOption(int argc, char *argv[], const char *option, const char *optDefault);
    static bool getBoolOption(int argc, char *argv[], const char *option);
};

#endif
