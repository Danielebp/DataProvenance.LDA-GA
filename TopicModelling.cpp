#include "TopicModelling.h"

using namespace std;

//#####################################################
//############## General Functions ####################
//#####################################################

TopicModelling::TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, long seed, ConfigOptions* cfg){
      this->numberOfTopics = numberOfTopics;
      this->numberOfIterations = numberOfIterations;
      this->seed = seed;
      this->numberOfDocuments = numberOfDocuments;
      this->cfg = cfg;
      switch (cfg->ldaLibrary) {
          case glda:
#if defined(USECUDA)
      		this->gldaModel = new model();
#endif
            break;
          case plda:
		this->PLDA_corpus = new vector<learning_lda::LDADocument*>();
      		this->PLDA_corpus->clear();
      		cfg->logger.log(debug, "Corpus has size " + to_string(PLDA_corpus->size()));
            break;
          case llda:
		this->lightldaModel = new multiverso::lightlda::LightLDA();
		this->LLDA_doc_index_map = new map<int, string>();
            break;
          case blda:
            blda_model = new LDA_Estimate(numberOfDocuments, numberOfTopics);
            break;
          default:
            break;
      }
  }

TopicModelling::~TopicModelling(){
      switch (cfg->ldaLibrary) {
          case glda:
#if defined(USECUDA)
           delete gldaModel;
#endif
            break;
          case plda:
            PLDA_FreeCorpus();
	    delete PLDA_accum_model;
            delete PLDA_word_index_map;
            break;
          case llda:
	    delete lightldaModel;
	    delete LLDA_doc_index_map;
	    break;
        case blda:
          delete blda_model;
          break;
        default:
                break;
      }
  }

int TopicModelling::getMainTopic(int docNum) {
  switch (cfg->ldaLibrary) {
    case llda:
      return lightldaModel->GetMainTopic(docNum);  
    case blda:
      return blda_model->getMainTopic(docNum); 
    default:
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
}

double TopicModelling::getDistribution(int topic, int docNum){
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_getDistribution(topic, docNum);
          case plda:
            return PLDA_getDistribution(topic, docNum);
          case llda:
            return lightldaModel->GetDocTopicDistribution(docNum, topic);
          case blda:
            return blda_model->getDocTopDist(docNum, topic);
          default:
            break;
      }
        return 0;
  }

 string TopicModelling::getDocNameByNumber(int num){
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_getDocNameByNumber(num);
          case plda:
            return PLDA_getDocNameByNumber(num);
          case llda:
            return (*LLDA_doc_index_map)[num];
          default:
            break;
      }
        return "";
  }
  void TopicModelling::WriteFiles(bool isfinal) {
      switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_WriteFiles(isfinal);
          case plda:
            return PLDA_WriteFiles(isfinal);
          case llda:
            return LLDA_WriteFiles(isfinal);
          case blda:
            return BLDA_WriteFiles(isfinal);
          default:
            break;
      }
  }

  long TopicModelling::LDA(string MyCount) {
     switch (cfg->ldaLibrary) {
          case glda:
            return GLDA_LDA(MyCount);
          case plda:
            return PLDA_LDA(MyCount);
          case llda:
            return LIGHT_LDA(MyCount);
          case blda:
            return BLDA_LDA(MyCount);
          default:
            break;
      }

      return 0;
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
    for(int docID = 0; docID<this->gldaModel->ptrndata->M; docID++){
        string name = this->gldaModel->ptrndata->docs[docID]->name;
        out<<name<<"\t";
        vector<pair<int, double> > topicsDistribution = this->gldaModel->getDocDistributions(docID);
        unsigned int count = 0;
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
  this->gldaModel->init(16, args, cfg);
  cfg->logger.log(debug, "GLDA setup completed. Starting estimate");

  if(cfg->perfType==cuda) this->gldaModel->cuda_estimate();
  else this->gldaModel->estimate();

  cfg->logger.log(debug, "GLDA estimate completed");

  // write topic.txt
  ofstream outTop;
  outTop.open (cfg->outputDir + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<this->gldaModel->getTopicDistribution(topic)<<"\t"<<this->gldaModel->maptopic2Words[topic]<<endl;
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
    dist = this->gldaModel->getDistribution(docNum, topic);
    #endif
    return dist;
}
string TopicModelling::GLDA_getDocNameByNumber(int num){
    string doc = "";
    #if defined(USECUDA)
    doc = this->gldaModel->getDocName(num);
    #endif
    return doc;
}

//#####################################################
//############## PLDA Functions #######################
//#####################################################

void TopicModelling::PLDA_WriteFiles(bool isfinal) {
    ofstream distFile(cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt");
    stringstream out;
    out<<"docID\ttopic\tdist\t..."<<endl;
    for (vector<learning_lda::LDADocument*>::iterator iter = PLDA_corpus->begin(); iter != PLDA_corpus->end(); ++iter) {
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
    cfg->logger.log(debug, "corpus has size: " + to_string(PLDA_corpus->size()));
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  long time = 0;
  outputFile = MyCount;
  cfg->logger.log(debug, "corpus has size: " + to_string(PLDA_corpus->size()));
  PLDA_LoadAndInitTrainingCorpus(cfg->ldaInputFile);
  cfg->logger.log(debug, "Finished Load -> Start Model");

  clock_t t = clock();
  learning_lda::LDAModel model(numberOfTopics, PLDA_word_index_map);
  PLDA_accum_model = new learning_lda::LDAAccumulativeModel(numberOfTopics, PLDA_word_index_map->size(), PLDA_corpus->size());

  cfg->logger.log(debug, "Finished Model -> Start Train");

  PLDA_TrainModel(&model); // corpus gets screwed here

  cfg->logger.log(debug, "Finished Train -> Write file");

  t = clock() - t;
  time = ((float)t)/(CLOCKS_PER_SEC/1000);

  // Show top 5 words in topics with proportions for the first document

//  cfg->logger.log(debug, "Finished Write -> Start Infer");
//  time += Infer(model, word_index_map, cfg->ldaInputFile);
//  cfg->logger.log(debug, "###########################");

  cfg->logger.log(debug, "#### Writing LDA ####");

  cfg->logger.log(debug, "word index map has size: " + to_string(PLDA_word_index_map->size()));
  // write topic.txt
  ofstream outTop;
  outTop.open (cfg->outputDir + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<PLDA_accum_model->GetGlobalTopicDistribution(topic)<<"\t"<<PLDA_accum_model->GetWordsPerTopic(PLDA_word_index_map, topic, 20, " ")<<endl;
  }
  outTop.close();

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}
double TopicModelling::PLDA_getDistribution(int topic, int docNum) {
    double dist = 0.0;

    dist = PLDA_accum_model->GetDocTopicDistribution(docNum, topic);
    if(dist<0 || dist>1)cfg->logger.log(error, "Got bad distribution: " + to_string(dist));

    return dist;
}

string TopicModelling::PLDA_getDocNameByNumber(int num){
    string doc = "";

    for (vector<learning_lda::LDADocument*>::iterator iter = PLDA_corpus->begin(); iter != PLDA_corpus->end(); ++iter) {
     if((*iter)->id == num) return (*iter)->name;
    }

    return doc;
}

bool TopicModelling::PLDA_TrainModel(learning_lda::LDAModel * model) {
    learning_lda::LDASampler sampler(0.01, 0.01, model, PLDA_accum_model);
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
    PLDA_accum_model->AverageModel(numberOfIterations - burnInIterations);

    return true;
}
int TopicModelling::PLDA_LoadAndInitTrainingCorpus(const string& corpus_file) {
  cfg->logger.log(debug, "Restart corpus");
  cfg->logger.log(debug, "Restart word index");
  PLDA_word_index_map = new map<string, int>();
  cfg->logger.log(debug, "Done");

  cfg->logger.log(debug, corpus_file);
  ifstream fin(corpus_file.c_str());
  cfg->logger.log(debug, "file loaded");
  string line;
  // TODO: move reading of files outside, and make the constructor receive the data already
  cfg->logger.log(debug, "Staring to read input file");
  int count = 0;
  map<string, int>* wordCount = new map<string, int>();
  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.

      unsigned int pos = line.find(cfg->delimiter);
      if(pos >= line.size()) cfg->logger.log(error, "Delimiter not found");
      string docName = line.substr(0, pos);
      cfg->logger.log(debug, "Read line: " + docName);
      line = line.substr(pos + 17);
      cfg->logger.log(debug, line);

      learning_lda::DocumentWordTopicsPB document;
      vector<int32> topics;
      int word_index;

      // agrupate tokens with count of repetitions
      wordCount->clear();
      PLDA_AgrupateTokens(line, wordCount);
      for (map<string, int>::iterator it = wordCount->begin(); it != wordCount->end(); it++ ) {
        topics.clear();

        // for (int i = 0; i < it->second; ++i) {
          topics.push_back(learning_lda::RandInt(numberOfTopics));
        // }

        map<string, int>::const_iterator iter = PLDA_word_index_map->find(it->first);
        if (iter == PLDA_word_index_map->end()) {
          word_index = PLDA_word_index_map->size();
          (*PLDA_word_index_map)[it->first] = word_index;
        } else {
          word_index = iter->second;
        }

        document.add_wordtopics(it->first, word_index, topics);
      }
      cfg->logger.log(debug, "Finished line: " + docName + " Corpus has size " + to_string(PLDA_corpus->size()));
      PLDA_corpus->push_back(new learning_lda::LDADocument(document, numberOfTopics, docName, count++));
    }
  }
  fin.close();
  delete wordCount;
  return PLDA_corpus->size();
}
bool TopicModelling::PLDA_AgrupateTokens (string line, map<string, int>* wordCount) {
  string word;
  stringstream ss(line);

  while (ss >> word) {
    map<string, int>::const_iterator iter = wordCount->find(word);
    if (iter == wordCount->end()) {
      (*wordCount)[word] = 1;
    } else {
      (*wordCount)[word] +=1;
    }
  }

  return true;
}
void TopicModelling::PLDA_FreeCorpus() {
  for (vector<learning_lda::LDADocument*>::iterator iter = PLDA_corpus->begin();
       iter != PLDA_corpus->end();
       ++iter) {
	delete (*iter);
	*iter = 0;
  }
}

// #######################################################
// ##################### LightLDA ########################
// #######################################################

long TopicModelling::LIGHT_LDA(string MyCount){

    srand(seed);
    cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                  + " topics and " + to_string(numberOfIterations) + " iterations ####");

    outputFile = MyCount;

    clock_t t = clock();

    string ntopics = to_string(numberOfTopics);
    string niters = to_string(numberOfIterations);
    string ndocs = to_string(numberOfDocuments);

    cfg->logger.log(debug, "Reading Doc Map: " + cfg->docmapFile);

    Scanner sc2;
    try {
        cfg->logger.log(debug, "Oppenning file");
        sc2.open(cfg->docmapFile);
	cfg->logger.log(debug, "Oppened file");
        do {
            int idx = sc2.nextInt();
            string name = sc2.nextWord();
           (*LLDA_doc_index_map)[idx] = name;
        } while(sc2.nextLine());
    } catch (exception& e) {

        cfg->logger.log(error, "Hit error while reading the Doc Map File");
        cfg->logger.log(error, e.what());
    }
    sc2.close();

    cfg->logger.log(debug, "Starting LightLDA estimate");
    // -num_vocabs 111400 -num_topics 1000 -num_iterations 2 -alpha 0.1 -beta 0.01 -mh_steps 2 -num_local_workers 1 -num_blocks 1 -max_num_document 300000 -input_dir ~/git_files/DataProvenance.LDA-GA/LightLDA/example/data/nytimes/ -data_capacity 800
    string num_vocab = "111400";
    char* args[] = {(char*)"-num_vocabs", (char*)(num_vocab.c_str()), //TODO: missing
                  (char*)"-num_topics", (char*)(ntopics.c_str()),
                  (char*)"-num_iterations", (char*)(niters.c_str()),
                  (char*)"-alpha", (char*)"0.1",
                  (char*)"-beta", (char*)"0.01",
                  (char*)"-mh_steps", (char*)"2",
                  (char*)"-num_local_workers", (char*)"1",
                  (char*)"-num_blocks", (char*)"1",
                  (char*)"-max_num_document", (char*)"300000", //TODO: missing
                  (char*)"-input_dir", const_cast<char*>(cfg->inputDir.c_str()),// TODO: missing
                  (char*)"-data_capacity", (char*)"800", //TODO: missing
                  NULL};
    lightldaModel->Run(22, args, cfg->outputDir);

    cfg->logger.log(debug, "LightLDA estimate completed");

    vector<double> LLDA_topicDist = lightldaModel->GetTopicDistributions(numberOfDocuments,numberOfTopics);
    cfg->logger.log(debug, "Got Topic Dist");
    // write topic.txt
    ofstream outTop;
    outTop.open (cfg->outputDir + "/topic.txt");
    outTop<<"topic\tdist\twords"<<endl;
    for (int topic = 0; topic < numberOfTopics; topic++) {
        outTop<<topic<<"\t"<<LLDA_topicDist[topic]<<"\t"<<"a b c d e f g h i j k l m n o p q r s t"<<endl;// TODO: fix words this->lightldaModel->maptopic2Words[topic]<<endl;
    }
    outTop.close();
    cfg->logger.log(debug, "Wrote Topic Dist");

    t = clock() - t;
    long time = (((float)t)/(CLOCKS_PER_SEC/1000));

    cfg->logger.log(debug, "#### Ending LDA ####");

    return time;
}

void TopicModelling::LLDA_WriteFiles(bool isfinal) {
    ofstream distFile(cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt");
    stringstream out;
    out<<"docID\ttopic\tdist\t..."<<endl;
    for (int doc = 0; doc<numberOfDocuments; doc++) {
        out<<(*LLDA_doc_index_map)[doc]<<"\t";
     for(int topic = 0; topic<numberOfTopics; topic++){
         out << topic << "\t" << lightldaModel->GetDocTopicDistribution(doc, topic)
         << ((topic < (numberOfTopics - 1)) ? "\t" : "\n");
     }
    }
    distFile<<out.str();;

    ofstream modelFile(cfg->outputDir + "/model"+(isfinal ? "" : outputFile)+".txt");
    stringstream fout;
    PLDA_accum_model->AppendAsString(PLDA_word_index_map, fout);
    modelFile<<fout.str();
}


// #######################################################
// ###################### BleiLDA ########################
// #######################################################
long TopicModelling::BLDA_LDA(string MyCount) {
  srand(seed);
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  outputFile = MyCount;

  clock_t t = clock();

  blda_corpus* ldacorpus = read_data(const_cast<char*>(cfg->libsvmFile.c_str()));
  cfg->logger.log(debug, "BLDA setup completed. Starting estimate");

  blda_model->runLDA(ldacorpus, numberOfTopics, numberOfIterations, 0.1, cfg->outputDir);

  cfg->logger.log(debug, "BLDA estimate completed");

  // write topic.txt
  ofstream outTop;
  outTop.open (cfg->outputDir + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<this->blda_model->getTopicDist(topic)<<"\t"<<"a b c d e f g h i j k l m n o p q r s t"<<endl;
  }

  outTop.close();
  t = clock() - t;
  long time = (((float)t)/(CLOCKS_PER_SEC/1000));

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}

void TopicModelling::BLDA_WriteFiles(bool isfinal) {
    ofstream distFile(cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt");
    stringstream out;
    out<<"docID\ttopic\tdist\t..."<<endl;
    for (int d=0; d<numberOfDocuments; d++) {
        out<<d<<"\t";
        for(int topic = 0; topic<numberOfTopics; topic++){
            out << topic << "\t" << blda_model->getDocTopDist(d, topic)
                << ((topic < (numberOfTopics - 1)) ? "\t" : "\n");
        }
    }
    distFile<<out.str();
}
