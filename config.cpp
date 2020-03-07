#include "config.h"

#include <sys/stat.h>
#include <sys/types.h>

struct stat sb;

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
    mutationLevel	= j["mutationLevel"];
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
    mutationLevel	= 0.7;
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

}

bool ConfigOptions::start() {
    cout<<"Start up dir.."<<endl;
    if(!(stat(outputDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) ){
        mkdir(outputDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cout<<"Create dir "<<outputDir<<endl;
    }
    if(!(stat(mirrorDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
        mkdir(mirrorDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cout<<"Create dir "<<mirrorDir<<endl;
    }
    cout<<"Start logger"<<endl;
    loggerFile = outputDir+"/"+loggerFile;
    logger.init(loggerFile, logLevel, logFileLevel);

    return true;
}

