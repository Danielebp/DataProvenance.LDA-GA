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
#if defined(USECUDA)
          case glda:
          case gibbs:
      		this->gldaModel = new model();
            break;
#endif
          case plda:
		this->PLDA_corpus = new vector<learning_lda::LDADocument*>();
      		this->PLDA_corpus->clear();
      		cfg->logger.log(debug, "Corpus has size " + to_string(PLDA_corpus->size()));
            break;
          case blda:
            blda_model = new LDA_Estimate(numberOfDocuments, numberOfTopics);
    		this->doc_index_map = new vector<string>();
            this->doc_index_map->reserve(numberOfDocuments);
            LoadDocMap();
            break;
          case wlda:
            this->doc_index_map = new vector<string>();
            this->doc_index_map->reserve(numberOfDocuments);
            LoadDocMap();
            wldaDocTopDist = new double*[numberOfDocuments];
            for(unsigned i = 0; i < numberOfDocuments; ++i)
                wldaDocTopDist[i] = new double[numberOfTopics];
          default:
            this->doc_index_map = new vector<string>();
            this->doc_index_map->reserve(numberOfDocuments);
            LoadDocMap();
            break;
      }
  }

TopicModelling::~TopicModelling(){
      switch (cfg->ldaLibrary) {
#if defined(USECUDA)
          case glda:
          case gibbs:
           delete gldaModel;
            break;
#endif
          case plda:
            PLDA_FreeCorpus();
	    delete PLDA_accum_model;
            delete PLDA_word_index_map;
            break;
        case blda:
          delete blda_model;
          delete doc_index_map;
          break;
        case wlda:
          delete doc_index_map;
          for(int i = 0; i < numberOfDocuments; ++i)
            delete [] wldaDocTopDist[i];
          delete [] wldaDocTopDist;
        default:
                break;
      }
  }

int TopicModelling::getMainTopic(int docNum) {
  switch (cfg->ldaLibrary) {
    case blda:
      return blda_model->getMainTopic(docNum);
    default:
      double max = 0.0;
      int idMax = -1;

      for(int i = 0; i<numberOfTopics; i++) {
          double curr = getDistribution (i, docNum);
        if(curr > max) {
            max = curr;
            idMax = i;
        }
      }
      return idMax;
  }
}

double TopicModelling::getDistribution(int topic, int docNum){
      switch (cfg->ldaLibrary) {
#if defined(USECUDA)
          case glda:
          case gibbs:
            return GLDA_getDistribution(topic, docNum);
#endif
          case plda:
            return PLDA_getDistribution(topic, docNum);
          case blda:
            return blda_model->getDocTopDist(docNum, topic);
          case wlda:
            return wldaDocTopDist[docNum][topic];
          default:
            break;
      }
        return 0;
  }

 string TopicModelling::getDocNameByNumber(int num){
      switch (cfg->ldaLibrary) {
#if defined(USECUDA)
          case glda:
          case gibbs:
            return GLDA_getDocNameByNumber(num);
#endif
          case plda:
            return PLDA_getDocNameByNumber(num);
          default:
            return (*doc_index_map)[num];
      }
        return "";
  }
  void TopicModelling::WriteFiles(bool isfinal) {
      switch (cfg->ldaLibrary) {
#if defined(USECUDA)
          case glda:
          case gibbs:
            return GLDA_WriteFiles(isfinal);
#endif
          case plda:
            return PLDA_WriteFiles(isfinal);
          case blda:
            return BLDA_WriteFiles(isfinal);
          case wlda:
            return WLDA_WriteFiles(isfinal);
          default:
            break;
      }
  }

  long TopicModelling::LDA(string MyCount) {
     switch (cfg->ldaLibrary) {
 #if defined(USECUDA)
          case glda:
          case gibbs:
            return GLDA_LDA(MyCount);
#endif
          case plda:
            return PLDA_LDA(MyCount);
          case blda:
            return BLDA_LDA(MyCount);
          case wlda:
            return WLDA_LDA(MyCount);
          default:
            break;
      }

      return 0;
  }

  bool TopicModelling::LoadDocMap(){
      cfg->logger.log(debug, "Reading Doc Map: " + cfg->ldaInputFile);
      string line;
      int docID = 0;
      ifstream ldaInputFile;
      ldaInputFile.open (cfg->ldaInputFile);
      while (getline(ldaInputFile, line)) {
          // Each line has top 20 words of a topic
          string name = line.substr(0, line.find(cfg->delimiter));
//          (*doc_index_map)[docID++] = name;
          doc_index_map->push_back(name);
      }
      ldaInputFile.close();

      return true;
}

//#####################################################
//############## GLDA Functions #######################
//#####################################################

#if defined(USECUDA)
void TopicModelling::GLDA_WriteFiles(bool isfinal) {
    string filename = cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt";

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

    //ofstream fout(cfg->outputDir + "/model"+outputFile+".txt");
    //fout<<res;
}
// should write distribution.txt and topics.txt
long TopicModelling::GLDA_LDA(string MyCount) {
  srand(seed);
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  outputFile = MyCount;

  Timer t;
  t.start();

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
  long time = t.getTime();

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}
double TopicModelling::GLDA_getDistribution(int topic, int docNum) {
    double dist = 0.0;
    dist = this->gldaModel->getDistribution(docNum, topic);
    return dist;
}
string TopicModelling::GLDA_getDocNameByNumber(int num){
    string doc = "";
    doc = this->gldaModel->getDocName(num);
    return doc;
}
#endif

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

  long ttime = 0;
  outputFile = MyCount;
  cfg->logger.log(debug, "corpus has size: " + to_string(PLDA_corpus->size()));
  PLDA_LoadAndInitTrainingCorpus(cfg->ldaInputFile);
  cfg->logger.log(debug, "Finished Load -> Start Model");

  Timer t;
  t.start();
  learning_lda::LDAModel model(numberOfTopics, PLDA_word_index_map);
  PLDA_accum_model = new learning_lda::LDAAccumulativeModel(numberOfTopics, PLDA_word_index_map->size(), PLDA_corpus->size());

  cfg->logger.log(debug, "Finished Model -> Start Train");

  PLDA_TrainModel(&model); // corpus gets screwed here

  cfg->logger.log(debug, "Finished Train -> Write file");

  ttime = t.getTime();

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

  return ttime;
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
// ###################### BleiLDA ########################
// #######################################################
long TopicModelling::BLDA_LDA(string MyCount) {
  srand(seed);
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  outputFile = MyCount;

  Timer t;
  t.start();

  blda_corpus* ldacorpus = read_data(const_cast<char*>(cfg->libsvmFile.c_str()));
  cfg->logger.log(debug, "BLDA setup completed. Starting estimate");

  blda_model->runLDA(ldacorpus, numberOfTopics, numberOfIterations, 0.1, cfg->outputDir);

  cfg->logger.log(debug, "BLDA estimate completed");

  Scanner sc2;
  try {
      cfg->logger.log(debug, "Oppenning file");
      sc2.open(cfg->docmapFile);
  cfg->logger.log(debug, "Oppened file");
      do {
          int idx = sc2.nextInt();
          string name = sc2.nextWord();
         (*doc_index_map)[idx] = name;
      } while(sc2.nextLine());
  } catch (exception& e) {

      cfg->logger.log(error, "Hit error while reading the Doc Map File");
      cfg->logger.log(error, e.what());
  }
  sc2.close();

  // write topic.txt
  ofstream outTop;
  outTop.open (cfg->outputDir + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<this->blda_model->getTopicDist(topic)<<"\t"<<"a b c d e f g h i j k l m n o p q r s t"<<endl;
  }

  outTop.close();
  long time = t.getTime();

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}

void TopicModelling::BLDA_WriteFiles(bool isfinal) {
    ofstream distFile(cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt");
    stringstream out;
    out<<"docID\ttopic\tdist\t..."<<endl;

    for (int d=0; d<numberOfDocuments; d++) {
        out<<(*doc_index_map)[d]<<"\t";
        for(int topic = 0; topic<numberOfTopics; topic++){
            out << topic << "\t" << blda_model->getDocTopDist(d, topic)
                << ((topic < (numberOfTopics - 1)) ? "\t" : "\n");
        }
    }
    distFile<<out.str();
}

// #######################################################
// ###################### WarpLDA ########################
// #######################################################
long TopicModelling::WLDA_LDA(string MyCount) {
  srand(seed);
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  outputFile = MyCount;

  Timer t;
  t.start();
  string winf = cfg->inputDir + "wlda_input";
  string ntopics = to_string(numberOfTopics);
  string niters = to_string(numberOfIterations);
  char* args[] = {(char*)"warplda",
                (char*)"--prefix", (char*)winf.c_str(),
                (char*)"--k", (char*)ntopics.c_str(),
                (char*)"--niter", (char*)niters.c_str(),
                (char*)"--dir", (char*)(cfg->outputDir.c_str()),
                 NULL};

  cfg->logger.log(debug, "WLDA setup completed. Starting estimate");

  run_wlda(9, args);

  cfg->logger.log(debug, "WLDA estimate completed");

  // load doc x topic distribution from prefix.dist files
  if(!WLDA_loadDistributions())
    cfg->logger.log(error, "Error reading Doc x Topic distributions");

  // load words from prefix.info.words.txt file
  vector<string> topWords;
  topWords.reserve(numberOfTopics);
  if(!WLDA_loadWords(&topWords))
    cfg->logger.log(error, "Error reading TopWords for topics");

  vector<double> topDist;
  topDist.reserve(numberOfTopics);
  // TODO: load topic distribution somehow
  for (int topic = 0; topic < numberOfTopics; topic++) {
      topDist.push_back(0.0);
  }

  // write topic.txt
  ofstream outTop;
  outTop.open (cfg->outputDir + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<topDist[topic]<<"\t"<<topWords[topic]<<endl;
  }

  outTop.close();
  long time = t.getTime();

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}

bool TopicModelling::WLDA_loadDistributions(){
    Scanner sc2;
    try {
        sc2.open(cfg->outputDir + "/wdistribution.txt");
        do {
            int docID = sc2.nextInt();
            for (unsigned t = 0; t<numberOfTopics; t++){
                int topic  = sc2.nextInt();
                double dist = sc2.nextDouble();
                wldaDocTopDist[docID][topic] = dist;
            }
        } while(sc2.nextLine());
    } catch (exception& e) {

        cfg->logger.log(error, "Hit error while reading the Distribution File");
        cfg->logger.log(error, e.what());
        return false;
    }
    sc2.close();
    return true;
}

bool TopicModelling::WLDA_loadWords(vector<string>* topWords){
    string line;
    try {
        ifstream topWordsFile;
        topWordsFile.open (cfg->outputDir + "/top_words.txt");
        while (getline(topWordsFile, line)) {
            // Each line has top 20 words of a topic
            topWords->push_back(line);
        }
        topWordsFile.close();
    } catch (exception& e) {
        cfg->logger.log(error, "Hit error while reading the Distribution File");
        cfg->logger.log(error, e.what());
        return false;
    }

    return true;
}

void TopicModelling::WLDA_WriteFiles(bool isfinal) {
    ifstream idFile(cfg->outputDir + "/wdistribution.txt");
    ofstream distFile(cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt");
    string line;
    while (getline(idFile, line)) {
        int pos = line.find("\t");
        int docID = stoi(line.substr(0, pos));
        string docName = getDocNameByNumber(docID);
        distFile<<docName<<line.substr(pos)<<endl;
    }
}
