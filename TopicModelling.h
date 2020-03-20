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
    model* gldaModel;
    #endif

    //#####################################################
    //################ PLDA Variables #####################
    //#####################################################
    learning_lda::LDAAccumulativeModel* PLDA_accum_model;
    map<string, int>* PLDA_word_index_map;


public:
    vector<learning_lda::LDADocument*>* PLDA_corpus;
  TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, long seed, ConfigOptions* cfg);

  ~TopicModelling();

  //#####################################################
  //############## General Functions ####################
  //#####################################################

  inline double getDistribution(int topic, int docNum){
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_getDistribution(topic, docNum);
          case plda:
            return PLDA_getDistribution(topic, docNum);
          case gibbslda:
	    return 0;
      }
	return 0;
  }
  inline string getDocNameByNumber(int num){
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_getDocNameByNumber(num);
          case plda:
            return PLDA_getDocNameByNumber(num);
	  case gibbslda:
	    return "";
      }
	return "";
  }
  inline void WriteFiles(bool isfinal) {
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_WriteFiles(isfinal);
          case plda:
            return PLDA_WriteFiles(isfinal);
	  case gibbslda:
	    return;
      }
  }

  inline long LDA(string MyCount = "") {
     cfg->logger.log(debug, "Corpus has size " + to_string(PLDA_corpus->size())); 
     switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_LDA(MyCount);
          case plda:
            return PLDA_LDA(MyCount);
	  case gibbslda:
	    return 0;
      }

      return 0;
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
  bool PLDA_AgrupateTokens (string line, map<string, int>* wordCount);
  void PLDA_FreeCorpus();

  //#####################################################
  //############## Gibbs LDA Functions ##################
  //#####################################################


};

#endif
