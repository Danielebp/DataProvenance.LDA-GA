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
#include "plda/common.h"
#include "plda/document.h"
#include "plda/model.h"
#include "plda/accumulative_model.h"
#include "plda/sampler.h"
#include "plda/cmd_flags.h"

using namespace std;


class TopicModelling {
private:

    //#####################################################
    //############## General Variables ####################
    //#####################################################

    string outputFile;
    ConfigOptions* cfg;
    int numberOfTopics;
    int numberOfIterations;
    int numberOfDocuments;
    long seed;


    //#####################################################
    //################ GLDA Variables #####################
    //#####################################################
    #if defined(USECUDA)
    model ldaModel;
    #endif

    //#####################################################
    //################ PLDA Variables #####################
    //#####################################################
    learning_lda::LDAAccumulativeModel* PLDA_accum_model;
    learning_lda::LDACorpus PLDA_corpus;
    map<string, int>* PLDA_word_index_map;


public:
  TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, long seed, ConfigOptions* cfg):PLDA_corpus(){
      this->numberOfTopics = numberOfTopics;
      this->numberOfIterations = numberOfIterations;
      this->seed = seed;
      this->numberOfDocuments = numberOfDocuments;
      this->cfg = cfg;
  }

  ~TopicModelling(){
      switch (cfg->ldaLibrary) {
          case glda:
            break;
          case plda:
            PLDA_FreeCorpus();
            break;
      }
  }

  //#####################################################
  //############## General Functions ####################
  //#####################################################

  inline double getDistribution(int topic, int docNum){
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_getDistribution(topic, docNum);
          case plda:
            return PLDA_getDistribution(topic, docNum);
      }
  }
  inline string getDocNameByNumber(int num){
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_getDocNameByNumber(num);
          case plda:
            return PLDA_getDocNameByNumber(num);
      }
  }
  inline void WriteFiles(bool isfinal) {
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_WriteFiles(isfinal);
          case plda:
            return PLDA_WriteFiles(isfinal);
      }
  }

  inline long LDA(string MyCount = "") {
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_LDA(MyCount);
          case plda:
            return PLDA_LDA(MyCount);
      }
  }

  int getMainTopic(int docNum);


  //#####################################################
  //############## GLDA Functions #######################
  //#####################################################
  double GLDA_getDistribution(int topic, int docNum);
  string GLDA_getDocNameByNumber(int num);
  void GLDA_WriteFiles(bool isfinal) ;
  long GLDA_LDA(string MyCount = "") ;

  //#####################################################
  //############## PLDA Functions #######################
  //#####################################################
  void PLDA_WriteFiles(bool isfinal) ;
  // should write distribution.txt and topics.txt
  long PLDA_LDA(string MyCount) ;
  double PLDA_getDistribution(int topic, int docNum);
  string PLDA_getDocNameByNumber(int num);
  bool PLDA_TrainModel(learning_lda::LDAModel * model) ;
  int PLDA_LoadAndInitTrainingCorpus(const string& corpus_file) ;
  map<string, int> PLDA_AgrupateTokens (string line);
  void PLDA_FreeCorpus();

  //#####################################################
  //############## Gibbs LDA Functions ##################
  //#####################################################


};

#endif
