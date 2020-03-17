#ifndef __config_h__
#define __config_h__

#include <fstream>
#include <iostream>
#include "json.hpp"
#include "logger.h"
#include "commons.h"

using json = nlohmann::json;
using namespace std;

NLOHMANN_JSON_SERIALIZE_ENUM(LogLevel, {{error, "error"},{status, "status"},{info, "info"}, {debug,"debug"}})
NLOHMANN_JSON_SERIALIZE_ENUM(RunType,  {{metric, "metric"}, {train,"train"}})
NLOHMANN_JSON_SERIALIZE_ENUM(PerfType, {{cpu, "cpu"}, {cuda,"cuda"}})
NLOHMANN_JSON_SERIALIZE_ENUM(LDALibrary, {{plda, "plda"}, {gibbslda,"gibbslda"}, {glda, "glda"}})


class ConfigOptions {

public:
    // running config
    LogLevel    logLevel;
    LogLevel    logFileLevel;
    RunType     runType;
    PerfType    perfType;
    LDALibrary  ldaLibrary;

    // parameters
    int     populationSize;
    int	    device;
    double  mutationLevel;
    double  fitnessThreshold;
    bool    skipPreprocess;

    // files and locations
    string  dataDir;//        = "txtData";        // name of the directory that contains the original source data
    string  mirrorDir;//      = "processedData";  // name of the directory where the modified data is to be stored
    string  outputDir;

    string  stopWordsFile;//  = "stopwords.txt";
    string  truthFile;//  = "truthfile.txt";
    string preProcessedFile;
    string  ldaInputFile;//   = "./tempData/input1.txt";
    string  loggerFile;//   = "log.txt";
    string  delimiter;//      = "##lda_delimiter##";

    Logger logger;

    ConfigOptions(string filename);
    bool start();
};

#endif
