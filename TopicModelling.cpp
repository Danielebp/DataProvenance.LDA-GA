#include "TopicModelling.h"

using namespace std;

/*
map<string, int> TopicModelling::AgrupateTokens (string line) {
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

void TopicModelling::FreeCorpus(LDACorpus* corpus) {
  for (list<LDADocument*>::iterator iter = corpus->begin();
       iter != corpus->end();
       ++iter) {
    if (*iter != NULL) {
      delete *iter;
      *iter = NULL;
    }
  }
}
*/
void TopicModelling::WriteFiles() {


    ofstream fout(cfg->outputDir + "/distribution" + outputFile + ".txt");
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
    fout<<out.str();;

    //ofstream fout(cfg->outputDir + "/model"+outputFile+".txt");
    //fout<<res;
}

// should write distribution.txt and topics.txt
long TopicModelling::LDA(string MyCount) {
// srand(10);
  stringstream ss;
  ss<<"#### Starting LDA with "<<numberOfTopics<<" topics and "<<numberOfIterations<<" iterations ####";
  cfg->logger.log(debug, ss.str());
    ss.str(std::string());
    ss.clear();

  outputFile = MyCount;

  clock_t t = clock();

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
  t = clock() - t;
  long time = (((float)t)/(CLOCKS_PER_SEC/1000));

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}

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
