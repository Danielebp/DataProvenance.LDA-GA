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
/*
int TopicModelling::LoadAndInitTrainingCorpus(const string& corpus_file, map<string, int>* word_index_map, LDACorpus* corpus) {
    if(debug) cout<<"Restart corpus"<<endl;
  corpus->clear();
  word_index_map->clear();

  ifstream fin(corpus_file.c_str());
  string line;
  // TODO: move reading of files outside, and make the constructor receive the data already
  if(debug) cout<<"Staring to read input file"<<endl;
  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.

      int pos = line.find(delimiter);
      if(pos >= line.size()) cout<<"Delimiter not found"<<endl;
      if(debug) cout<<"Read line: "<< line.substr(0, pos)<<endl;
      line.erase(0, pos + 17);

      DocumentWordTopicsPB document;
      vector<int32> topics;
      int word_index;

      // agrupate tokens with count of repetitions
      map<string, int> wordCount = AgrupateTokens(line);
      for (map<string, int>::iterator it = wordCount.begin(); it != wordCount.end(); it++ ) {
        topics.clear();

        // for (int i = 0; i < it->second; ++i) {
          topics.push_back(RandInt(numberOfTopics));
        // }

        map<string, int>::const_iterator iter = word_index_map->find(it->first);
        if (iter == word_index_map->end()) {
          word_index = word_index_map->size();
          (*word_index_map)[it->first] = word_index;
        } else {
          word_index = iter->second;
        }

        document.add_wordtopics(it->first, word_index, topics);
      }
      corpus->push_back(new LDADocument(document, numberOfTopics));
    }
  }
  return corpus->size();
}

LDAAccumulativeModel TopicModelling::TrainModel(LDAModel * model, LDACorpus & corpus, int wordIndexMapSize) {
    LDAAccumulativeModel accum_model(numberOfTopics, wordIndexMapSize);
    LDASampler sampler(0.01, 0.01, model, &accum_model);

    sampler.InitModelGivenTopics(corpus);

    for (int iter = 0; iter < numberOfIterations; ++iter) {
      if(debug) cout << "Iteration " << iter << " ...\n";
      // if (flags.compute_likelihood_ == "true") {
      //   double loglikelihood = 0;
      //   for (list<LDADocument*>::const_iterator iterator = corpus.begin(); iterator != corpus.end(); ++iterator) {
      //     loglikelihood += sampler.LogLikelihood(*iterator);
      //   }
      //   cout << "Loglikelihood: " << loglikelihood << endl;
      // }
      sampler.DoIteration(&corpus, true, iter < burnInIterations);
    }
    accum_model.AverageModel(numberOfIterations - burnInIterations);

    FreeCorpus(&corpus);

    return accum_model;
}

// TODO: distribution is wrong
long TopicModelling::Infer(LDAModel model, map<string, int> word_index_map, string inputFile, string header) {
  LDASampler sampler(0.01, 0.01, &model, NULL);
  ifstream fin(inputFile);
  stringstream out;
  out<<header<<endl;
  string line;
  string docID;
  int docNum = 0;
  clock_t t;
  long time = 0;
  // TODO: move read files outside
  while (getline(fin, line)) {  // Each line is a training document.
    if(debug) cout<<"Read line"<<endl;
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.

      if(debug) cout<<"Line not empty"<<endl;
      DocumentWordTopicsPB document_topics;

      int pos = line.find(delimiter);
      docID = line.substr(0, pos);
      line.erase(0, pos + 17);
      if(debug) cout<<"Remove delimiter"<<endl;

      // agrupate tokens with count of repetitions
      map<string, int> wordCount = AgrupateTokens(line);
      if(debug) cout<<"Aggrupate tokens"<<endl;

      for (map<string, int>::iterator it = wordCount.begin(); it != wordCount.end(); it++ ){
        vector<int32> topics;
        // for (int i = 0; i < it->second; ++i) {
          topics.push_back(RandInt(model.num_topics()));
        // }
        map<string, int>::const_iterator iter = word_index_map.find(it->first);
        if (iter != word_index_map.end()) {
          document_topics.add_wordtopics(it->first, iter->second, topics);
        }
      }
      if(debug) cout<<"Assigned topics?"<<endl;

      t = clock();

      LDADocument document(document_topics, model.num_topics());
      TopicProbDistribution prob_dist(model.num_topics(), 0);
      for (int iteration = 0; iteration < numberOfIterations; ++iteration) {
        sampler.SampleNewTopicsForDocument(&document, false);
        if (iteration >= burnInIterations) {
          const vector<int64>& document_distribution =
              document.topic_distribution();
          for (int i = 0; i < document_distribution.size(); ++i) {
            prob_dist[i] += document_distribution[i];
          }
        }
      }
      if(debug) cout<<"Ran iterations"<<endl;

      t = clock() - t;

      out<<docID<<"\t";
      docsMap[docNum] = docID;
      for (int topic = 0; topic < prob_dist.size(); ++topic) {
        distribution[((docNum)*numberOfTopics) + topic] = prob_dist[topic] / (numberOfIterations - burnInIterations);
        out << topic << "\t" << distribution[((docNum)*numberOfTopics) + topic]
            << ((topic < prob_dist.size() - 1) ? "\t" : "\n");
      }
      if(debug) cout<<"Save distributions"<<endl;

      time += ((float)t)/(CLOCKS_PER_SEC/1000);
      docNum++;
    }
  }
  if(debug) cout<<"Finished reading file"<<endl;

  dist = out.str();

  double total = 0;
  for (int i = 0; i < numberOfTopics; i++) {
      topicDistribution[i] = 0;
      for (int j = 0; j < numberOfDocuments; j++) {
          topicDistribution[i] += distribution[((j)*numberOfTopics) + i];
          total += distribution[((j)*numberOfTopics) + i];
      }
  }
  for (int i = 0; i < numberOfTopics; i++) {
      topicDistribution[i] = topicDistribution[i]/total;
  }
  if(debug) cout<<"Wrote total distribution"<<endl;

  return time;
}
*/

// should write distribution.txt and topics.txt
long TopicModelling::LDA(string MyCount) {
// srand(10);
  if(debug) cout<<"#### Starting LDA with "<<numberOfTopics<<" topics and "<<numberOfIterations<<" iterations ####"<<endl;
  long time = 0;
  string inputFile = "./tempData/input1.txt";
  outputFile = MyCount;

  char* args[] = {(char*)"lda", (char*)"-est",
                (char*)"-ntopics", const_cast<char*>(std::to_string(numberOfTopics).c_str()),
                (char*)"-niters", const_cast<char*>(std::to_string(numberOfIterations).c_str()),
                (char*)"-twords", (char*)"20",
                (char*)"-dfile", const_cast<char*>(inputFile.c_str()),
                (char*)"-ndocs", (char*)"131",
                (char*)"-savestep", (char*)"0",
                (char*)"-dir", (char*)"./tempData/",
                NULL};

  this->ldaModel.init(16, args);
  this->ldaModel.estimate();


  // write topic.txt
  ofstream outTop;
  outTop.open ("./tempData/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<this->ldaModel.getTopicDistribution(topic)<<"\t"<<this->ldaModel.maptopic2Words[topic]<<endl;
  }

  outTop.close();


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
