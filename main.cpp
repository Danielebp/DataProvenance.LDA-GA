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

void CheckLDAPerformance(int numberOfDocuments, ConfigOptions cfg);

int main(int argc, char* argv[]) {
    // fixing seed for testing purposes
    srand(time(NULL));
    string configFile = "";

    // should be multiple of 3
    int arg=1;
    if(argc == 2){
        configFile = argv[arg++];
    }
    ConfigOptions cfg(configFile);

    for (; arg < argc; arg++) {
        string s = argv[arg];
        if(s.compare("-p") == 0)
            cfg.populationSize = stoi(argv[++arg]);
        else if(s.compare("-f") == 0)
            cfg.fitnessThreshold = stod(argv[++arg]);
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


    cout<<endl;

    unordered_map<string, Document> documentsMap = preProcess();

    if(cfg.runType == metric) {
      CheckLDAPerformance(documentsMap.size(), cfg);
    }
    else {
      // call genetic logic to perform LDA-GA
      reconstructProvenance(populationSize, documentsMap.size(), fitnessThreshold, cfg);

    }
}

void CheckLDAPerformance(int numberOfDocuments, ConfigOptions cfg) {
    int TEST_COUNT = 3;
    long LDATotTime = 0;
    string line;
    int tpcs[] = {2, 4, 6, 8, 10};
    int times[5][5];

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
            cfg.logger.log(info, number_of_topics<<"x"<<number_of_iterations<<": " + to_string(popCfg.LDA_execution_milliseconds)<<"ms");
        }
    }
    cfg.logger.log(info, "topics\t");
    for(int i=100; i<=500; i+=100){
      cfg.logger.log(info, i<<(i==500 ? "\n" : "\t"));
}
    for(int i=0; i<5; i++){
      cfg.logger.log(info, tpcs[i]<<"\t");
      for(int j=0; j<5; j++){
        cfg.logger.log(info, times[i][j]<<(j==4 ? "\n":"\t"));
}
}

}
