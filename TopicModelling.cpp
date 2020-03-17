#include "TopicModelling.h"

using namespace std;

//#####################################################
//############## General Functions ####################
//#####################################################

int TopicModelling::getMainTopic(int docNum) {
    double max = 0.0;
    int idMax = -1;

    for(int i = 0; i<numberOfTopics; i++) {
        if(getDistribution (i, docNum) > max) {
            max = getDistribution (i, docNum);
            idMax = i;
        }
    }

    return idMax;
}



//#####################################################
//############## GLDA Functions #######################
//#####################################################

void TopicModelling::GLDA_WriteFiles(bool isfinal) {
    string filename = cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt";

    #if defined(USECUDA)
    ofstream fout(filename);
    stringstream out;
    out<<"docID\ttopic\tdist\t..."<<endl;
    for(int docID = 0; docID<this->ldaModel.ptrndata->M; docID++){
        string name = this->ldaModel.ptrndata->docs[docID]->name;
        out<<name<<"\t";
        vector<pair<int, double> > topicsDistribution = this->ldaModel.getDocDistributions(docID);
        int count = 0;
        for(auto t=topicsDistribution.begin(); t!=topicsDistribution.end(); ++t){
          out << t->first << "\t" << t->second
              << ((count < topicsDistribution.size() - 1) ? "\t" : "\n");
          count++;
        }
    }
    fout<<out.str();
    #endif

    //ofstream fout(cfg->outputDir + "/model"+outputFile+".txt");
    //fout<<res;
}
// should write distribution.txt and topics.txt
long TopicModelling::GLDA_LDA(string MyCount) {
  srand(seed);
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  outputFile = MyCount;

  clock_t t = clock();
  #if defined(USECUDA)

  string ntopics = to_string(numberOfTopics);
  string niters = to_string(numberOfIterations);
  string ndocs = to_string(numberOfDocuments);

  char* args[] = {(char*)"lda", (char*)"-est",
                (char*)"-ntopics", (char*)(ntopics.c_str()),
                (char*)"-niters", (char*)(niters.c_str()),
                (char*)"-twords", (char*)"20",
                (char*)"-dfile", const_cast<char*>(cfg->ldaInputFile.c_str()),
                (char*)"-ndocs", const_cast<char*>(ndocs.c_str()),
                (char*)"-savestep", (char*)"0",
                (char*)"-dir", const_cast<char*>(cfg->outputDir.c_str()),
                NULL};
  this->ldaModel.init(16, args, cfg);
  cfg->logger.log(debug, "GLDA setup completed. Starting estimate");

  if(cfg->perfType==cuda) this->ldaModel.cuda_estimate();
  else this->ldaModel.estimate();

  cfg->logger.log(debug, "GLDA estimate completed");

  // write topic.txt
  ofstream outTop;
  outTop.open (cfg->outputDir + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<this->ldaModel.getTopicDistribution(topic)<<"\t"<<this->ldaModel.maptopic2Words[topic]<<endl;
  }

  outTop.close();
  #endif
  t = clock() - t;
  long time = (((float)t)/(CLOCKS_PER_SEC/1000));

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}
double TopicModelling::GLDA_getDistribution(int topic, int docNum) {
    double dist = 0.0;
    #if defined(USECUDA)
    dist = this->ldaModel.getDistribution(docNum, topic);
    #endif
    return dist;
}
string TopicModelling::GLDA_getDocNameByNumber(int num){
    string doc = "";
    #if defined(USECUDA)
    doc = this->ldaModel.getDocName(num);
    #endif
    return doc;
}

//#####################################################
//############## PLDA Functions #######################
//#####################################################

// TODO: stopped here
void TopicModelling::PLDA_WriteFiles(bool isfinal) {
    ofstream distFile(cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt");
    stringstream out;
    out<<"docID\ttopic\tdist\t..."<<endl;
    for (list<learning_lda::LDADocument*>::iterator iter = PLDA_corpus->begin(); iter != PLDA_corpus->end(); ++iter) {
     out<<(*iter)->name<<"\t";
     for(int topic = 0; topic<numberOfTopics; topic++){
         out << topic << "\t" << PLDA_accum_model->GetDocTopicDistribution((*iter)->id, topic)
         << ((topic < (numberOfTopics - 1)) ? "\t" : "\n");
     }
    }
    distFile<<out.str();;

    ofstream modelFile(cfg->outputDir + "/model"+(isfinal ? "" : outputFile)+".txt");
    stringstream fout;
    PLDA_accum_model->AppendAsString(PLDA_word_index_map, fout);
    modelFile<<fout.str();
}
// should write distribution.txt and topics.txt
long TopicModelling::PLDA_LDA(string MyCount) {
  srand(seed);
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  long time = 0;
  outputFile = MyCount;

  int docCount = PLDA_LoadAndInitTrainingCorpus(cfg->ldaInputFile);

  cfg->logger.log(debug, "Finished Load -> Start Model");

  clock_t t = clock();
  learning_lda::LDAModel model(numberOfTopics, PLDA_word_index_map);

  cfg->logger.log(debug, "Finished Model -> Start Train");

  PLDA_accum_model = PLDA_TrainModel(&model);

  cfg->logger.log(debug, "Finished Train -> Write file");

  t = clock() - t;
  time = ((float)t)/(CLOCKS_PER_SEC/1000);

  // Show top 5 words in topics with proportions for the first document

//  cfg->logger.log(debug, "Finished Write -> Start Infer");
//  time += Infer(model, word_index_map, cfg->ldaInputFile);
//  cfg->logger.log(debug, "###########################");

  cfg->logger.log(debug, "#### Writing LDA ####");
  // write topic.txt
  ofstream outTop;
  outTop.open (cfg->outputDir + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<PLDA_accum_model->GetGlobalTopicDistribution()[topic]<<"\t"<<PLDA_accum_model->GetWordsPerTopic(PLDA_word_index_map, topic, 20, " ")<<endl;
  }
  outTop.close();

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}
double TopicModelling::PLDA_getDistribution(int topic, int docNum) {
    double dist = 0.0;

    dist = PLDA_accum_model->GetDocTopicDistribution(docNum, topic);

    return dist;
}
learning_lda::LDAAccumulativeModel* TopicModelling::PLDA_TrainModel(learning_lda::LDAModel * model) {
    learning_lda::LDAAccumulativeModel accum_model(numberOfTopics, PLDA_word_index_map.size(), PLDA_corpus->size());
    learning_lda::LDASampler sampler(0.01, 0.01, model, &accum_model);
    int burnInIterations = 10;

    sampler.InitModelGivenTopics(PLDA_corpus);

    for (int iter = 0; iter < numberOfIterations; ++iter) {
      cfg->logger.log(debug, "Iteration " + to_string(iter));
      // if (flags.compute_likelihood_ == "true") {
      //   double loglikelihood = 0;
      //   for (list<LDADocument*>::const_iterator iterator = PLDA_corpus->begin(); iterator != PLDA_corpus->end(); ++iterator) {
      //     loglikelihood += sampler.LogLikelihood(*iterator);
      //   }
      //   cout << "Loglikelihood: " << loglikelihood << endl;
      // }
      sampler.DoIteration(PLDA_corpus, true, iter < burnInIterations);
    }
    accum_model.AverageModel(numberOfIterations - burnInIterations);

    PLDA_FreeCorpus();

    return &accum_model;
}
int TopicModelling::PLDA_LoadAndInitTrainingCorpus(const string& corpus_file) {
  cfg->logger.log(debug, "Restart corpus");
  PLDA_corpus->clear();
  PLDA_word_index_map.clear();

  ifstream fin(corpus_file.c_str());
  string line;
  // TODO: move reading of files outside, and make the constructor receive the data already
  cfg->logger.log(debug, "Staring to read input file");
  int count = 0;
  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.

      int pos = line.find(cfg->delimiter);
      if(pos >= line.size()) cout<<"Delimiter not found"<<endl;
      string docName = line.substr(0, pos);
      cfg->logger.log(debug, "Read line: " + docName);
      line.erase(0, pos + 17);

      learning_lda::DocumentWordTopicsPB document;
      vector<int32> topics;
      int word_index;

      // agrupate tokens with count of repetitions
      map<string, int> wordCount = PLDA_AgrupateTokens(line);
      for (map<string, int>::iterator it = wordCount.begin(); it != wordCount.end(); it++ ) {
        topics.clear();

        // for (int i = 0; i < it->second; ++i) {
          topics.push_back(learning_lda::RandInt(numberOfTopics));
        // }

        map<string, int>::const_iterator iter = PLDA_word_index_map.find(it->first);
        if (iter == PLDA_word_index_map.end()) {
          word_index = PLDA_word_index_map.size();
          PLDA_word_index_map[it->first] = word_index;
        } else {
          word_index = iter->second;
        }

        document.add_wordtopics(it->first, word_index, topics);
      }
      PLDA_corpus->push_back(new learning_lda::LDADocument(document, numberOfTopics));
      PLDA_corpus->back()->name = docName;
      PLDA_corpus->back()->id = count++;
    }
  }
  return PLDA_corpus->size();
}
map<string, int> TopicModelling::PLDA_AgrupateTokens (string line) {
  map<string, int> wordCount;
  string word;
  stringstream ss(line);

  while (ss >> word) {
    map<string, int>::const_iterator iter = wordCount.find(word);
    if (iter == wordCount.end()) {
      (wordCount)[word] = 1;
    } else {
      (wordCount)[word] +=1;
    }
  }

  return wordCount;
}
void TopicModelling::PLDA_FreeCorpus() {
  for (list<learning_lda::LDADocument*>::iterator iter = PLDA_corpus->begin();
       iter != PLDA_corpus->end();
       ++iter) {
    if (*iter != NULL) {
      delete *iter;
      *iter = NULL;
    }
  }
}
