#ifndef __logger_h__
#define __logger_h__

#include <fstream>
#include <iostream>
#include "commons.h"

using namespace std;



class Logger {

public:
    // running config
    LogLevel    logLevel;
    ofstream logFile;

    Logger(string filename, LogLevel level);
    ~Logger();
    bool log(LogLevel level, string message);

};

#endif
