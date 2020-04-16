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
    ldaLibrary = j["ldaLibrary"];
    skipPreprocess = j["skipPreprocess"];

    populationSize      = j["populationSize"];
    device		= j["device"];
    mutationLevel	= j["mutationLevel"];
    fitnessThreshold    = j["fitnessThreshold"];

    dataDir     = j["dataDir"];
    mirrorDir   = j["mirrorDir"];
    outputDir   = j["outputDir"];

    inputDir     = j["inputDir"];
    wordmapFile  = j["wordmapFile"];
    docmapFile  = j["docmapFile"];
    libsvmFile   = j["libsvmFile"];

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
    ldaLibrary = glda;

    populationSize      = 9;
    device		= 0;
    fitnessThreshold    = 0.8;
    mutationLevel	= 0.7;
    skipPreprocess      = false;

    dataDir     = "txtData";
    mirrorDir   = "processedData";
    outputDir   = "tempData";

    inputDir     = "./data/data_10/";
    wordmapFile  = "input1.word_id.dict";
    docmapFile  = "input1.id_doc.dict";
    libsvmFile   = "input1.libsvm";

    stopWordsFile = "stopwords.txt";
    loggerFile    = "log.txt";
    truthFile     = "truthfile.txt";
    ldaInputFile  = "input1.txt";
    preProcessedFile = "input0.txt";
    delimiter     = "##lda_delimiter##";
    }

}

bool ConfigOptions::start() {
    
    truthFile       = inputDir + truthFile;
    dataDir         = inputDir + dataDir;
    preProcessedFile= inputDir + preProcessedFile;
    mirrorDir       = inputDir + mirrorDir;
    ldaInputFile    = inputDir + ldaInputFile;
    outputDir       = inputDir + outputDir;
    wordmapFile     = inputDir + wordmapFile;
    docmapFile     = inputDir + docmapFile;
    libsvmFile      = inputDir + libsvmFile;

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
