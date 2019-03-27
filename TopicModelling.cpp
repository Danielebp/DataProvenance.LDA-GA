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

int TopicModelling::LoadAndInitTrainingCorpus(const string& corpus_file, int num_topics, map<string, int>* word_index_map, LDACorpus* corpus) {
  corpus->clear();
  word_index_map->clear();

  ifstream fin(corpus_file.c_str());
  string line;
  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.

      //convert all the letters to lowercase
      transform(line.begin(), line.end(), line.begin(), ::tolower);
      int pos = line.find("##LDA_DELIMITER##");
      line.erase(0, pos + 17);

      DocumentWordTopicsPB document;
      vector<int32> topics;
      int word_index;

      // agrupate tokens with count of repetitions
      map<string, int> wordCount = AgrupateTokens(line);
      for (map<string, int>::iterator it = wordCount.begin(); it != wordCount.end(); it++ ) {
        topics.clear();

        for (int i = 0; i < it->second; ++i) {
          topics.push_back(RandInt(num_topics));
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
      corpus->push_back(new LDADocument(document, num_topics));
    }
  }
  return corpus->size();
}

void TopicModelling::Infer(LDAModel model, map<string, int> word_index_map, string inputFile, string outputFile, string header, int numberOfIterations, int burn_in_iterations) {
  LDASampler sampler(0.01, 0.01, &model, NULL);
  ifstream fin(inputFile);
  ofstream out(outputFile);
  out<<header<<endl;
  string line;
  string docID;
  while (getline(fin, line)) {  // Each line is a training document.
    if (line.size() > 0 &&      // Skip empty lines.
        line[0] != '\r' &&      // Skip empty lines.
        line[0] != '\n' &&      // Skip empty lines.
        line[0] != '#') {       // Skip comment lines.
      DocumentWordTopicsPB document_topics;

      int pos = line.find("##LDA_DELIMITER##");
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
      for (int iter = 0; iter < numberOfIterations; ++iter) {
        sampler.SampleNewTopicsForDocument(&document, false);
        if (iter >= burn_in_iterations) {
          const vector<int64>& document_distribution =
              document.topic_distribution();
          for (int i = 0; i < document_distribution.size(); ++i) {
            prob_dist[i] += document_distribution[i];
          }
        }
      }

      out<<docID<<"\t";
      for (int topic = 0; topic < prob_dist.size(); ++topic) {
        out << prob_dist[topic] /
              (numberOfIterations - burn_in_iterations)
            << ((topic < prob_dist.size() - 1) ? "\t" : "\n");
      }
    }
  }
}

void TopicModelling::LDA(int numberOfTopics, int numberOfIterations, bool topicFile, string MyCount) {
  srand(time(NULL));
  string inputFile = "input1.txt";
  string outputFile = "distribution" + MyCount + ".txt";
  LDACorpus corpus;
  map<string, int> word_index_map;

  // TODO: double check burn in concept
  int burn_in_iterations = 2*numberOfIterations/3;
  int docCount = LoadAndInitTrainingCorpus(inputFile, numberOfTopics, &word_index_map, &corpus);

  LDAModel model(numberOfTopics, word_index_map);
  LDAAccumulativeModel accum_model(numberOfTopics, word_index_map.size());
  LDASampler sampler(0.01, 0.01, &model, &accum_model);

  sampler.InitModelGivenTopics(corpus);

  for (int iter = 0; iter < numberOfIterations; ++iter) {
    cout << "Iteration " << iter << " ...\n";
    // if (flags.compute_likelihood_ == "true") {
    //   double loglikelihood = 0;
    //   for (list<LDADocument*>::const_iterator iterator = corpus.begin(); iterator != corpus.end(); ++iterator) {
    //     loglikelihood += sampler.LogLikelihood(*iterator);
    //   }
    //   cout << "Loglikelihood: " << loglikelihood << endl;
    // }
    sampler.DoIteration(&corpus, true, iter < burn_in_iterations);
  }
  accum_model.AverageModel(numberOfIterations - burn_in_iterations);

  FreeCorpus(&corpus);

  ofstream fout("model"+MyCount+".txt");
  accum_model.AppendAsString(word_index_map, fout);

  // Show top 5 words in topics with proportions for the first document
  string header = "Document";
  for (int topic = 0; topic < numberOfTopics; topic++) {
    header += "\t" + accum_model.GetWordsPerTopic(word_index_map, topic, 10);
  }

  // infers
  Infer(model, word_index_map, inputFile, outputFile, header, numberOfIterations, burn_in_iterations);




}
