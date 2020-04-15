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
#if defined(USELLDA)
#include "LightLDA/src/lightlda.h"
#endif
#include "BleiLDA/lda-estimate.h"
#include "WarpLDA/src/warp.hpp"

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
    map<int, string>* doc_index_map;


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
#if defined(USELLDA)
    multiverso::lightlda::LightLDA* lightldaModel;
#endif
    //#####################################################
    //############### BleiLDA Variables ###################
    //#####################################################
    LDA_Estimate* blda_model;

    //#####################################################
    //############### WarpLDA Variables ###################
    //#####################################################
    double** wldaDocTopDist;

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
  bool LoadDocMap();

  //#####################################################
  //############## GLDA Functions #######################
  //#####################################################
#if defined(USECUDA)
  double GLDA_getDistribution(int topic, int docNum);
  string GLDA_getDocNameByNumber(int num);
  void GLDA_WriteFiles(bool isfinal) ;
  long GLDA_LDA(string MyCount = "") ;
#endif

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
  #if defined(USELLDA)
  void LLDA_WriteFiles(bool isfinal) ;
  long LIGHT_LDA(string MyCount);
  double LLDA_getDistribution(int topic);
  #endif

  //#####################################################
  //############### BleiLDA Functions ##################
  //#####################################################
  long BLDA_LDA(string MyCount);
  void BLDA_WriteFiles(bool isfinal) ;

  //#####################################################
  //################ WarpLDA Functions ##################
  //#####################################################
  long WLDA_LDA(string MyCount);
  bool WLDA_loadDistributions();
  bool WLDA_loadWords(vector<string>* topWords);
  void WLDA_WriteFiles(bool isfinal);
};

#endif
