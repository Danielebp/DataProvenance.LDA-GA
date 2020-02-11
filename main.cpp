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

using namespace std;

void CheckLDAPerformance(int numberOfDocuments, bool debug);

int main(int argc, char* argv[]) {
    // fixing seed for testing purposes
    srand(time(NULL));

    // should be multiple of 3
    int populationSize = 9;
    double fitnessThreshold = 0.85;
    bool metrics = false;
    bool debug = false;
    bool progress = false;

    for (int i = 1; i < argc; i++) {
        string s = argv[i];
        if(s.compare("-p") == 0)
            populationSize = stoi(argv[++i]);
        else if(s.compare("-f") == 0)
            fitnessThreshold = stod(argv[++i]);
        else if(s.compare("-metrics") == 0)
            metrics = true;
        else if(s.compare("-debug") == 0)
            debug = true;
        else if(s.compare("-progress") == 0)
            progress = true;
        else
            cout<<"\tparameter not recognized: "<<argv[i]<<endl;
    }
    cout<<endl;

    unordered_map<string, Document> documentsMap = preProcess();

    if(metrics) {
      CheckLDAPerformance(documentsMap.size(), debug);
    }
    else {
      // call genetic logic to perform LDA-GA
      reconstructProvenance(populationSize, documentsMap.size(), fitnessThreshold, debug, progress);

    }
}

void CheckLDAPerformance(int numberOfDocuments, bool debug) {
    int TEST_COUNT = 3;
    long LDATotTime = 0;
    string line;
    int tpcs[] = {20};


    for (int i = 0; i <= 3; i++) {
        int number_of_topics = tpcs[i];
		for (int number_of_iterations = 100; number_of_iterations <= 1000; number_of_iterations +=100) {
            PopulationConfig popCfg;
            popCfg.number_of_topics = number_of_topics;
            popCfg.number_of_iterations = number_of_iterations;
            LDATotTime = 0;

            for (int i = 0; i < TEST_COUNT; ++i) {
                TopicModelling tm(number_of_topics, number_of_iterations, numberOfDocuments, debug);
                string id = "__"+to_string(i/2)+"__"+to_string(number_of_topics)+"x"+to_string(number_of_iterations);

                LDATotTime += tm.LDA(id);
            }
            popCfg.LDA_execution_milliseconds = ((double)LDATotTime/TEST_COUNT);
            cout<< number_of_topics<<"x"<<number_of_iterations<<": " + to_string(popCfg.LDA_execution_milliseconds)<<"ms"<<endl;
        }
    }
}
