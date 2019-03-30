#ifndef __TOPIC_MODELLING_H__
#define __TOPIC_MODELLING_H__

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>

#include "utils.h"
#include "plda/common.h"
#include "plda/document.h"
#include "plda/model.h"
#include "plda/accumulative_model.h"
#include "plda/sampler.h"
#include "plda/cmd_flags.h"

using namespace std;
using namespace learning_lda;

class TopicModelling {
private:
    string delimiter = "##lda_delimiter##";
    double *distribution;
    unordered_map<int,string> docsMap;

    int numberOfTopics;
    int numberOfIterations;
    int burnInIterations;
    int numberOfDocuments;

public:
  TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments){
      distribution = new double[numberOfTopics*numberOfDocuments];
      this->numberOfTopics = numberOfTopics;
      this->numberOfIterations = numberOfIterations;
      this->burnInIterations = (2*numberOfIterations)/3;
      this->numberOfDocuments = numberOfDocuments;
  }

  ~TopicModelling(){
      delete[] distribution;
  }

  inline double getDistribution(int topic, int docNum) {
      return distribution[((docNum++)*numberOfTopics) + topic];
  }

  inline string getDocNameByNumber(int num){
      return docsMap[num];
  }

  int getMainTopic(int docNum);

  map<string, int> AgrupateTokens (string line) ;

  void FreeCorpus(LDACorpus* corpus) ;

  int LoadAndInitTrainingCorpus(const string& corpus_file, map<string, int>* word_index_map, LDACorpus* corpus) ;

  void Infer(LDAModel model, map<string, int> word_index_map, string inputFile, string outputFile, string header) ;

  LDAAccumulativeModel TrainModel(LDAModel * model, LDACorpus & corpus, int wordIndexMapSize) ;

  void LDA(string MyCount = "") ;

};

#endif
