#include "TopicModelling.h"

using namespace std;
using namespace learning_lda;


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

void TopicModelling::WriteFiles() {


    ofstream out("results/distribution" + outputFile + ".txt");
    out<<dist;

    ofstream fout("results/model"+outputFile+".txt");
    fout<<res;
}

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


// should write distribution.txt and topics.txt
long TopicModelling::LDA(string MyCount) {
// srand(10);
  if(debug) cout<<"#### Starting LDA ####"<<endl;
  long time = 0;
  string inputFile = "input1.txt";
  outputFile = MyCount;
  LDACorpus corpus;
  map<string, int> word_index_map;

  int docCount = LoadAndInitTrainingCorpus(inputFile, &word_index_map, &corpus);

  if(debug) cout<<"Finished Load -> Start Model"<<endl;

  clock_t t = clock();
  LDAModel model(numberOfTopics, word_index_map);

  if(debug) cout<<"Finished Model -> Start Train"<<endl;

  LDAAccumulativeModel accum_model = TrainModel(&model, corpus, word_index_map.size());

  if(debug) cout<<"Finished Train -> Write file"<<endl;

  stringstream fout;
  accum_model.AppendAsString(word_index_map, fout);
  res = fout.str();
  // Show top 5 words in topics with proportions for the first document
  string header = "Document\tTopic\tTopic Proportion\t...";

  if(debug) cout<<"Finished Write -> Start Infer"<<endl;
  t = clock() - t;
  time = ((float)t)/(CLOCKS_PER_SEC/1000);
  // infers
  time += Infer(model, word_index_map, inputFile, header);
  if(debug) cout<<"###########################"<<endl;

  if(debug) cout<<"#### Writing LDA ####"<<endl;

  // write topic.txt
  ofstream outTop;
  outTop.open ("results/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<topicDistribution[topic]<<"\t"<<accum_model.GetWordsPerTopic(word_index_map, topic, 20, " ")<<endl;
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
