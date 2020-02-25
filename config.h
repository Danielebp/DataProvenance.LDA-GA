#ifndef __config_h__
#define __config_h__

#include <fstream>
#include <iostream>
#include "json.hpp"
#include "logger.h"
#include "commons.h"

using json = nlohmann::json;
using namespace std;

NLOHMANN_JSON_SERIALIZE_ENUM(LogLevel, {{error, "error"},{info, "info"}, {debug,"debug"}})
NLOHMANN_JSON_SERIALIZE_ENUM(RunType,  {{metric, "metric"}, {train,"train"}})
NLOHMANN_JSON_SERIALIZE_ENUM(PerfType, {{cpu, "cpu"}, {cuda,"cuda"}})


class ConfigOptions {

public:
    // running config
    LogLevel    logLevel;
    RunType     runType;
    PerfType    perfType;

    // parameters
    int     populationSize;
    double  fitnessThreshold;

    // files and locations
    string  dataDir;//        = "txtData";        // name of the directory that contains the original source data
    string  mirrorDir;//      = "processedData";  // name of the directory where the modified data is to be stored
    string  outputDir;

    string  stopWordsFile;//  = "stopwords.txt";
    string  truthFile;//  = "truthfile.txt";
    string  ldaInputFile;//   = "./tempData/input1.txt";
    string  loggerFile;//   = "./tempData/input1.txt";
    string  delimiter;//      = "##lda_delimiter##";

    Logger logger;

    ConfigOptions(string filename);

};

#endif
