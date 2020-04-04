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
#include "scanner.h"

#if defined(USECUDA)
#include "gldaCuda/src/model.h"
#endif
#include "plda/common.h"
#include "plda/document.h"
#include "plda/model.h"
#include "plda/accumulative_model.h"
#include "plda/sampler.h"
#include "plda/cmd_flags.h"
#include "LightLDA/src/lightlda.h"
#include "BleiLDA/lda-estimate.h"

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

    //#####################################################
    //############### LightLDA Variables ##################
    //#####################################################
    multiverso::lightlda::LightLDA* lightldaModel;
    map<int, string>* LLDA_doc_index_map;
public:
    vector<learning_lda::LDADocument*>* PLDA_corpus;
  TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, long seed, ConfigOptions* cfg);

  ~TopicModelling();

  //#####################################################
  //############## General Functions ####################
  //#####################################################

  double getDistribution(int topic, int docNum);
  string getDocNameByNumber(int num);
  void WriteFiles(bool isfinal);
  long LDA(string MyCount = "");
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
  //############### LIGHTLDA Functions ##################
  //#####################################################
  void LLDA_WriteFiles(bool isfinal) ;
  long LIGHT_LDA(string MyCount);
  double LLDA_getDistribution(int topic);
};

#endif
