/**
 * Name: Chenying Wang
 * Email: chenying.wang@usc.edu
 * USC ID: ****-****-**
 * Date: Friday, January 24, 2020
 **/
#ifndef __UTILS
#define __UTILS

class Utils {
public:
    static char *getOption(int argc, char *argv[], const char *option, const char *optDefault);
    static bool getBoolOption(int argc, char *argv[], const char *option);
};

#endif
