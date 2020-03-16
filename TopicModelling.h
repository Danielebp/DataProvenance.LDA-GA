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
#include "config.h"
#include "utils.h"

#if defined(USECUDA)
#include "gldaCuda/src/model.h"
#endif


using namespace std;

class TopicModelling {
private:
    string outputFile;

    ConfigOptions* cfg;
    int numberOfTopics;
    int numberOfIterations;
    int numberOfDocuments;
    long seed;

    #if defined(USECUDA)
    model ldaModel;
    #endif

public:
  TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, long seed, ConfigOptions* cfg){
      this->numberOfTopics = numberOfTopics;
      this->numberOfIterations = numberOfIterations;
      this->seed = seed;
      this->numberOfDocuments = numberOfDocuments;
      this->cfg = cfg;
  }

  ~TopicModelling(){
  }

  inline double getDistribution(int topic, int docNum) {
      double dist = 0.0;
      #if defined(USECUDA)
      dist = this->ldaModel.getDistribution(docNum, topic);
      #endif
      return dist;
  }

  inline string getDocNameByNumber(int num){
      string doc = "";
      #if defined(USECUDA)
      doc = this->ldaModel.getDocName(num);
      #endif
      return doc;
  }

  void WriteFiles(bool isfinal) ;

  int getMainTopic(int docNum);

  //map<string, int> AgrupateTokens (string line) ;

  //void FreeCorpus(LDACorpus* corpus) ;

  long LDA(string MyCount = "") ;

};

#endif
