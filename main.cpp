#include <iostream>
#include <time.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "document.h"
#include "utils.h"
#include "dataProvenance.h"
#include "preProcessing.h"
#include "config.h"

using namespace std;

void CheckLDAPerformance(int numberOfDocuments, ConfigOptions* cfg);

int main(int argc, char* argv[]) {
    // fixing seed for testing purposes
    srand(time(NULL));
    string configFile = "";

    // should be multiple of 3
    int arg=1;
    if(argc >= 2){
        configFile = argv[arg++];
    }
    ConfigOptions cfg(configFile);

    cout<<"Log level: "<<getLogLabel(cfg.logLevel)<<endl;

    for (; arg < argc; arg++) {
        string s = argv[arg];
        if(s.compare("-p") == 0)
            cfg.populationSize = stoi(argv[++arg]);
        else if(s.compare("-m")==0)
            cfg.mutationLevel = stod(argv[++arg]);
	else if(s.compare("-f") == 0)
            cfg.fitnessThreshold = stod(argv[++arg]);
        else if(s.compare("-dir")==0){
	    string dir = argv[++arg];
            cfg.truthFile	= dir + cfg.truthFile; 
            cfg.dataDir		= dir + cfg.dataDir;
            cfg.preProcessedFile= dir + cfg.preProcessedFile;
            cfg.mirrorDir	= dir + cfg.mirrorDir;
            cfg.ldaInputFile	= dir + cfg.ldaInputFile;
            cfg.outputDir	= dir + cfg.outputDir;
	}
        else if(s.compare("-out")==0){
	    string outdir = argv[++arg];
            cfg.outputDir = cfg.outputDir + "/" + outdir;
	}
        else if(s.compare("-seed") == 0)
            srand(stoi(argv[++arg]));
        else if(s.compare("-cuda") == 0)
            cfg.perfType = cuda;
        else if(s.compare("-metrics") == 0)
            cfg.runType = metric;
        else if(s.compare("-debug") == 0)
            cfg.logLevel = debug;
        else if(s.compare("-info") == 0)
          cfg.logLevel = info;
        else
            cout<<"\tparameter not recognized: "<<argv[arg]<<endl;
    }
    unordered_map<string, Document> documentsMap;

    cfg.start();

    cfg.logger.log(debug, "Loading Dataset");
    if(cfg.skipPreprocess){
        cfg.logger.log(debug, "Skipped preprocess using " + cfg.preProcessedFile);
        documentsMap = loadPreProcessed(&cfg);
        cfg.logger.log(debug, "Loaded "+std::to_string(documentsMap.size())+" documents");
    }
    else{
       cfg.logger.log(debug, "Starting preprocess");
       documentsMap = preProcess(&cfg);
    }

    cfg.logger.log(status, "Running Provenance with the following configuration:");
    cfg.logger.log(status, "Fitness Threshold: " + std::to_string(cfg.fitnessThreshold));
    cfg.logger.log(status, "Population Size: " + std::to_string(cfg.populationSize));
    cfg.logger.log(status, "Running on "+ std::string(cfg.perfType == cuda ? "GPU" : "CPU"));
    cfg.logger.log(status, "Number of Documents: " + std::to_string(documentsMap.size()));

    int articlesCount = 0;
    for (std::pair<std::string, Document> element : documentsMap)
    {
        if (element.first.find("$AAA$") != string::npos) articlesCount++;
    }
    cfg.logger.log(status, "Number of Articles: " + std::to_string(articlesCount));

    if(cfg.runType == metric) {
      cfg.logger.log(debug, "Starting LDA performance test");
      CheckLDAPerformance(documentsMap.size(), &cfg);
    }
    else {
      cfg.logger.log(debug, "Starting provenance");
      // call genetic logic to perform LDA-GA
      reconstructProvenance(documentsMap.size(), &cfg);

    }
}

void CheckLDAPerformance(int numberOfDocuments, ConfigOptions *cfg) {
    int TEST_COUNT = 3;
    long LDATotTime = 0;
    string line;
    int tpcs[] = {2, 4, 6, 8, 10};
    int times[5][5];

    stringstream ss;
    for (int i = 0; i < 5; i++) {
        int number_of_topics = tpcs[i];
		for (int j = 0; j <5; j++) {
            int number_of_iterations = (j+1)*100;
            PopulationConfig popCfg;
            popCfg.number_of_topics = number_of_topics;
            popCfg.number_of_iterations = number_of_iterations;
            LDATotTime = 0;

            for (int i = 0; i < TEST_COUNT; ++i) {
                TopicModelling tm(number_of_topics, number_of_iterations, numberOfDocuments, cfg);
                string id = "__"+to_string(i/2)+"__"+to_string(number_of_topics)+"x"+to_string(number_of_iterations);

                LDATotTime += tm.LDA(id);
            }
            popCfg.LDA_execution_milliseconds = ((double)LDATotTime/TEST_COUNT);
            times[i][j] = popCfg.LDA_execution_milliseconds;
            ss<<number_of_topics<<"x"<<number_of_iterations<<": " + to_string(popCfg.LDA_execution_milliseconds)<<"ms";
            cfg->logger.log(info, ss.str());
            ss.str(std::string());
            ss.clear();
        }
    }

    ss<<"topics\t";
    for(int i=100; i<=500; i+=100){
      ss<<i<<(i==500 ? "\n" : "\t");
    }
    for(int i=0; i<5; i++){
      ss<<tpcs[i]<<"\t";
      for(int j=0; j<5; j++){
        ss<<times[i][j]<<(j==4 ? "\n":"\t");
      }
    }

    cfg->logger.log(status, ss.str());
    ss.str(std::string());
    ss.clear();
}
