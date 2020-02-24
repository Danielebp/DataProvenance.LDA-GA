#include <config.h>

enum LogLevel { error, info, debug };
enum RunType { metric, train };
enum PerfType { cpu, cuda };

    // running config
//    LogLevel logLevel;
//    RunType runType;
//    PerfType perfType;


public:
ConfigOptions::ConfigOptions(string filename){
    ifstream inputFile(filename);
    json j;
    inputFile >> j;

    populationSize      = j["populationSize"];
    fitnessThreshold    = j["fitnessThreshold"];

    dataDir     = j["dataDir"];
    mirrorDir   = j["mirrorDir"];
    outputDir   = j["outputDir"];

    stopWordsFile = j["stopWordsFile"];
    ldaInputFile  = j["ldaInputFile"];
    delimiter     = j["delimiter"];


}
