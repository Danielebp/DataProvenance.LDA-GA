#include "TopicModelling.h"

using namespace std;


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

/*void TopicModelling::FreeCorpus(LDACorpus* corpus) {
  for (list<LDADocument*>::iterator iter = corpus->begin();
       iter != corpus->end();
       ++iter) {
    if (*iter != NULL) {
      delete *iter;
      *iter = NULL;
    }
  }
}*/

void TopicModelling::WriteFiles() {


    ofstream out("results/distribution" + outputFile + ".txt");
    out<<dist;

    ofstream fout("results/model"+outputFile+".txt");
    fout<<res;
}

// Creates corpus of documents, where each document becomes a list of pairs wordIdx:count
int TopicModelling::LoadAndInitTrainingCorpus(const string& corpus_file, map<string, int>* word_index_map, corpus& c) {

  ifstream fin(corpus_file.c_str());
  string line;
  int nd = 0;

  c.docs = 0;
  c.num_terms = 0;
  c.num_docs = 0;

  // TODO: move reading of files outside, and make the constructor receive the data already
  if(debug) cout<<"Staring to read input file"<<endl;
  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.

        c.docs = (document*) realloc(c.docs, sizeof(document)*(nd+1));

        int pos = line.find(delimiter);
        if(pos >= line.size()) cout<<"Delimiter not found"<<endl;
        if(debug) cout<<"Read line: "<< line.substr(0, pos)<<endl;
        line.erase(0, pos + 17);

        // agrupate tokens on doc with count of repetitions
        map<string, int> wordCount = AgrupateTokens(line);

        // add initial doc information to corpus
        int numWords = wordCount.size();
        c.docs[nd].length = numWords;
        c.docs[nd].total = 0;
        c.docs[nd].words = (int*)malloc(sizeof(int)*numWords);
        c.docs[nd].counts = (int*)malloc(sizeof(int)*numWords);

        int n =0;
        for (map<string, int>::iterator wordCountIt = wordCount.begin();
                        wordCountIt != wordCount.end(); wordCountIt++ , n++) {

            // get word and count from map
            string word = wordCountIt->first;
            int count = wordCountIt->second;

            // makes sure word is on index map, if not add it
            map<string, int>::const_iterator idxIter = word_index_map->find(word);
            if (idxIter == word_index_map->end()) {
                (*word_index_map)[word] = word_index_map->size();
            }

            // add word idx to doc[nd]
            c.docs[nd].words[n] = (*word_index_map)[word];

            // add word count to doc[nd]
            c.docs[nd].counts[n] = count;
            c.docs[nd].total += count;
        }
        nd++;
    }
  }


  c.num_docs = nd;
  c.num_terms = word_index_map->size();

  return nd;
}

/*LDAAccumulativeModel TopicModelling::TrainModel(LDAModel * model, LDACorpus & corpus, int wordIndexMapSize) {
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
*/
// TODO: distribution is wrong
/*long TopicModelling::Infer(LDAModel model, map<string, int> word_index_map, string inputFile, string header) {
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
  if(debug) cout<<"#### Starting LDA ####"<<endl;
  long time = 0;
  string inputFile = "input1.txt";
  outputFile = MyCount;
  corpus corpus;
  map<string, int> word_index_map;
  LDA_Estimate ldaRunner;

  int docCount = LoadAndInitTrainingCorpus(inputFile, &word_index_map, corpus);

  if(debug) cout<<"Finished Loading "<<docCount<<"docs  -> Start Model"<<endl;

  clock_t t = clock();

  char* settingsFile="settings.txt";
  int modelCount = ldaRunner.runLDA(&corpus, numberOfTopics, 0.5f);

  t = clock() - t;
  time = ((float)t)/(CLOCKS_PER_SEC/1000);
  // infers

  if(debug) cout<<"#### Writing LDA ####"<<endl;
  cin>>inputFile;
  // write topic.txt
  ofstream outTop;
  outTop.open ("results/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      //outTop<<topic<<"\t"<<topicDistribution[topic]<<"\t"<<accum_model.GetWordsPerTopic(word_index_map, topic, 20, " ")<<endl;
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
