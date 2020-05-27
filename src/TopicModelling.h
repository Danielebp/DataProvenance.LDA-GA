#ifndef __TOPIC_MODELLING_H__
#define __TOPIC_MODELLING_H__

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <set>
#include <map>
#include "config.h"
#include "utils.h"
#include "scanner.h"
#include "timer.h"

#if defined(USECUDA)
#include "./LDA_Libraries/gldaCuda/src/model.h"
#endif
#include "./LDA_Libraries/plda/common.h"
#include "./LDA_Libraries/plda/document.h"
#include "./LDA_Libraries/plda/model.h"
#include "./LDA_Libraries/plda/accumulative_model.h"
#include "./LDA_Libraries/plda/sampler.h"
#include "./LDA_Libraries/plda/cmd_flags.h"
#include "./LDA_Libraries/BleiLDA/lda-estimate.h"
#include "./LDA_Libraries/WarpLDA/src/warp.hpp"

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
    vector<string>* doc_index_map;


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
    vector<learning_lda::LDADocument*>* PLDA_corpus;

    //#####################################################
    //############### BleiLDA Variables ###################
    //#####################################################
    LDA_Estimate* blda_model;

    //#####################################################
    //############### WarpLDA Variables ###################
    //#####################################################
    double** wldaDocTopDist;

public:
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
