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


    ofstream fout("./tempData/distribution" + outputFile + ".txt");
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

    //ofstream fout("./tempData/model"+outputFile+".txt");
    //fout<<res;
}

// should write distribution.txt and topics.txt
long TopicModelling::LDA(string MyCount) {
// srand(10);
  if(debug) cout<<"#### Starting LDA with "<<numberOfTopics<<" topics and "<<numberOfIterations<<" iterations ####"<<endl;
  string inputFile = "./tempData/input1.txt";
  outputFile = MyCount;

  clock_t t = clock();

  string ntopics = to_string(numberOfTopics);
  string niters = to_string(numberOfIterations);

  char* args[] = {(char*)"lda", (char*)"-est",
                (char*)"-ntopics", (char*)(ntopics.c_str()),
                (char*)"-niters", (char*)(niters.c_str()),
                (char*)"-twords", (char*)"20",
                (char*)"-dfile", const_cast<char*>(inputFile.c_str()),
                (char*)"-ndocs", (char*)"131",
                (char*)"-savestep", (char*)"0",
                (char*)"-dir", (char*)"./tempData/",
                NULL};
  this->ldaModel.init(16, args);
  if(debug) cout<<"GLDA setup completed. Starting estimate"<<endl;
  this->ldaModel.estimate();
  if(debug) cout<<"GLDA estimate completed"<<endl;

  // write topic.txt
  ofstream outTop;
  outTop.open ("./tempData/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<this->ldaModel.getTopicDistribution(topic)<<"\t"<<this->ldaModel.maptopic2Words[topic]<<endl;
  }

  outTop.close();
  t = clock() - t;
  long time = (((float)t)/(CLOCKS_PER_SEC/1000));

  if(debug) cout<<"#### Ending LDA ####"<<endl;

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
