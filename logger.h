#ifndef __logger_h__
#define __logger_h__

#include <fstream>
#include <iostream>
#include <chrono> 
#include <ctime>
#include "commons.h"

using namespace std;



class Logger {

public:
    // running config
    LogLevel logLevel;
    ofstream logFile;
    
    Logger() {logLevel = info;}
    Logger(string filename, LogLevel level);
    ~Logger();
    
    bool log(LogLevel level, string message);
    bool init(string filename, LogLevel level);
};

#endif
