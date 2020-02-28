#include "config.h"

//enum LogLevel { error, info, debug };
//enum RunType { metric, train };
//enum PerfType { cpu, cuda };

    // running config
//    LogLevel logLevel;
//    RunType runType;
//    PerfType perfType;


ConfigOptions::ConfigOptions(string filename){
    ifstream inputFile(filename);
    if(inputFile.good()){
    json j;
    inputFile >> j;

    logLevel = j["logLevel"];
    logFileLevel = j["logFileLevel"];
    runType = j["runType"];
    perfType = j["perfType"];
    skipPreprocess = j["skipPreprocess"];

    populationSize      = j["populationSize"];
    fitnessThreshold    = j["fitnessThreshold"];

    dataDir     = j["dataDir"];
    mirrorDir   = j["mirrorDir"];
    outputDir   = j["outputDir"];

    stopWordsFile = j["stopWordsFile"];
    truthFile     = j["truthFile"];
    ldaInputFile  = j["ldaInputFile"];
    preProcessedFile = j["preProcessedFile"];
    loggerFile    = j["loggerFile"];
    delimiter     = j["delimiter"];
    } else {

    logLevel = info;
    logFileLevel = status;
    runType = train;
    perfType = cpu;

    populationSize      = 9;
    fitnessThreshold    = 0.8;
    skipPreprocess      = false;
    
    dataDir     = "txtData";
    mirrorDir   = "processedData";
    outputDir   = "tempData";

    stopWordsFile = "stopwords.txt";
    loggerFile    = "log.txt";
    truthFile     = "truthfile.txt";
    ldaInputFile  = "input1.txt";
    preProcessedFile = "input0.txt";
    delimiter     = "##lda_delimiter##";
    }

    logger.init(loggerFile, logLevel, logFileLevel);
}
