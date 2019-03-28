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

int TopicModelling::LoadAndInitTrainingCorpus(const string& corpus_file, map<string, int>* word_index_map, LDACorpus* corpus) {
  corpus->clear();
  word_index_map->clear();

  ifstream fin(corpus_file.c_str());
  string line;
  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.

      int pos = line.find(delimiter);
      line.erase(0, pos + 17);

      DocumentWordTopicsPB document;
      vector<int32> topics;
      int word_index;

      // agrupate tokens with count of repetitions
      map<string, int> wordCount = AgrupateTokens(line);
      for (map<string, int>::iterator it = wordCount.begin(); it != wordCount.end(); it++ ) {
        topics.clear();

        for (int i = 0; i < it->second; ++i) {
          topics.push_back(RandInt(numberOfTopics));
        }

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

    cout<<"\tRunning "<<numberOfIterations<<" iterations for "<<numberOfTopics<<" topics..."<<endl;
    for (int iter = 0; iter < numberOfIterations; ++iter) {
      // cout << "Iteration " << iter << " ...\n";
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

void TopicModelling::Infer(LDAModel model, map<string, int> word_index_map, string inputFile, string outputFile, string header) {
  LDASampler sampler(0.01, 0.01, &model, NULL);
  ifstream fin(inputFile);
  ofstream out(outputFile);
  out<<header<<endl;
  string line;
  string docID;
  int docNum = 0;

  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.
      DocumentWordTopicsPB document_topics;

      int pos = line.find(delimiter);
      docID = line.substr(0, pos);
      line.erase(0, pos + 17);

      // agrupate tokens with count of repetitions
      map<string, int> wordCount = AgrupateTokens(line);
      for (map<string, int>::iterator it = wordCount.begin(); it != wordCount.end(); it++ ){
        vector<int32> topics;
        for (int i = 0; i < it->second; ++i) {
          topics.push_back(RandInt(model.num_topics()));
        }
        map<string, int>::const_iterator iter = word_index_map.find(it->first);
        if (iter != word_index_map.end()) {
          document_topics.add_wordtopics(it->first, iter->second, topics);
        }
      }
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

      out<<docID<<"\t";
      for (int topic = 0; topic < prob_dist.size(); ++topic) {
        distribution[((docNum)*numberOfTopics) + topic] = prob_dist[topic] / (numberOfIterations - burnInIterations);
        out << distribution[((docNum)*numberOfTopics) + topic]
            << ((topic < prob_dist.size() - 1) ? "\t" : "\n");
      }
      docNum++;
    }
  }
}

void TopicModelling::LDA(string MyCount) {
    srand(1);
  string inputFile = "input1.txt";
  string outputFile = "distribution" + MyCount + ".txt";
  LDACorpus corpus;
  map<string, int> word_index_map;

  int docCount = LoadAndInitTrainingCorpus(inputFile, &word_index_map, &corpus);

  // cout<<"Finished Load -> Start Model"<<endl;

  LDAModel model(numberOfTopics, word_index_map);

  // cout<<"Finished Model -> Start Train"<<endl;

  LDAAccumulativeModel accum_model = TrainModel(&model, corpus, word_index_map.size());

  // cout<<"Finished Train -> Write file"<<endl;

  ofstream fout("model"+MyCount+".txt");
  accum_model.AppendAsString(word_index_map, fout);

  // Show top 5 words in topics with proportions for the first document
  string header = "Document";
  for (int topic = 0; topic < numberOfTopics; topic++) {
    header += "\t" + accum_model.GetWordsPerTopic(word_index_map, topic, 10);
  }

  // cout<<"Finished Write -> Start Infer"<<endl;

  // infers
  Infer(model, word_index_map, inputFile, outputFile, header);

  // cout<<"###########################"<<endl;

}

int TopicModelling::getMainTopic(int docNum) {
    double max = 0.0;
    int idMax;

    for(int i = 0; i<numberOfTopics; i++) {
        if(getDistribution (i, docNum) > max) {
            max = getDistribution (i, docNum);
            idMax = i;
        }
    }

    return idMax;
}
