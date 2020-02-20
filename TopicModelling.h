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
#include "gldaCuda/src/model.h"

using namespace std;

class TopicModelling {
private:
    string delimiter = "##lda_delimiter##";
    string outputFile;
    string res;
    string dist;
    double *distribution;
    double *topicDistribution;
    unordered_map<int,string> docsMap;

    bool debug;
    bool cuda;
    int numberOfTopics;
    int numberOfIterations;
    int burnInIterations;
    int numberOfDocuments;
    model ldaModel;

public:
  TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, bool cuda,  bool debug){
      distribution = new double[numberOfTopics*numberOfDocuments];
      topicDistribution = new double[numberOfTopics];
      this->numberOfTopics = numberOfTopics;
      this->numberOfIterations = numberOfIterations;
      this->burnInIterations = (2*numberOfIterations)/3;
      this->numberOfDocuments = numberOfDocuments;
      this->cuda = cuda;
      this->debug = debug;
  }

  ~TopicModelling(){
      delete[] distribution;
      delete[] topicDistribution;
  }

  inline double getDistribution(int topic, int docNum) {
      return this->ldaModel.getDistribution(docNum, topic);
  }

  inline string getDocNameByNumber(int num){
      return docsMap[num];
  }

  void WriteFiles() ;

  int getMainTopic(int docNum);

  //map<string, int> AgrupateTokens (string line) ;

  //void FreeCorpus(LDACorpus* corpus) ;

  long LDA(string MyCount = "") ;

};

#endif
