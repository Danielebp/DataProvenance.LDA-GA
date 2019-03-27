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
    string delimiter = "##LDA_DELIMITER##";

public:
  TopicModelling(){}

  map<string, int> AgrupateTokens (string line) ;

  void FreeCorpus(LDACorpus* corpus) ;

  int LoadAndInitTrainingCorpus(const string& corpus_file, int num_topics, map<string, int>* word_index_map, LDACorpus* corpus) ;

  void Infer(LDAModel model, map<string, int> word_index_map, string inputFile, string outputFile, string header, int numberOfIterations, int burn_in_iterations) ;

  void LDA(int numberOfTopics, int numberOfIterations, bool topicFile, string MyCount = "") ;

};

#endif