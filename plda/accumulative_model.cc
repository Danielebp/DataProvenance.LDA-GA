// Copyright 2008 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "accumulative_model.h"

#include <algorithm>
#include <functional>
#include <map>
#include <numeric>
#include <string>

namespace learning_lda {

LDAAccumulativeModel::LDAAccumulativeModel(int num_topics, int vocab_size, int corpus_size) {
  CHECK_LT(1, num_topics);
  CHECK_LT(1, vocab_size);
  global_distribution_.resize(num_topics, 0);
  zero_distribution_.resize(num_topics, 0);
  topic_distributions_.resize(vocab_size);
  doc_distributions_.resize(corpus_size);
  for (int i = 0; i < vocab_size; ++i) {
    topic_distributions_[i].resize(num_topics, 0);
  }
  for (int i = 0; i < corpus_size; ++i) {
    doc_distributions_[i].resize(num_topics, 0);
  }
}

// Accumulate a model into accumulative_topic_distributions_ and
// accumulative_global_distributions_.
void LDAAccumulativeModel::AccumulateModel(const LDAModel& source_model, LDACorpus* corpus) {
  CHECK_EQ(num_topics(), source_model.num_topics());
  for (LDAModel::Iterator iter(&source_model); !iter.Done(); iter.Next()) {
    const TopicCountDistribution& source_dist = iter.Distribution();
    TopicProbDistribution* dest_dist = &(topic_distributions_[iter.Word()]);
    CHECK_EQ(num_topics(), source_dist.size());
    for (int k = 0; k < num_topics(); ++k) {
      (*dest_dist)[k] += static_cast<double>(source_dist[k]);
    }
  }
  for (int k = 0; k < num_topics(); ++k) {
    global_distribution_[k] +=
        static_cast<double>(source_model.GetGlobalTopicDistribution()[k]);
  }
  for (list<LDADocument*>::iterator iter = corpus->begin();
       iter != corpus->end();
       ++iter) {
       for (int k = 0; k < num_topics(); ++k) {
           doc_distributions_[(*iter)->id][k] += (*iter)->topic_distribution()[k];
       }
  }
}

void LDAAccumulativeModel::AverageModel(int num_accumulations) {
  for (vector<TopicProbDistribution>::iterator iter =
           topic_distributions_.begin();
       iter != topic_distributions_.end();
       ++iter) {
    TopicProbDistribution& dist = *iter;
    for (int k = 0; k < num_topics(); ++k) {
      dist[k] /= num_accumulations;
    }
  }

  for (vector<TopicProbDistribution>::iterator iter =
           doc_distributions_.begin();
       iter != doc_distributions_.end();
       ++iter) {
    TopicProbDistribution& dist = *iter;
    for (int k = 0; k < num_topics(); ++k) {
      dist[k] /= num_accumulations;
    }
  }

  for (int k = 0; k < num_topics(); ++k) {
    global_distribution_[k] /= num_accumulations;
  }
}

const TopicProbDistribution& LDAAccumulativeModel::GetWordTopicDistribution(
    int word) const {
  return topic_distributions_[word];
}

double LDAAccumulativeModel::GetDocTopicDistribution(
    int doc, int topic) const {
  return doc_distributions_[doc][topic];
}

const TopicProbDistribution&
LDAAccumulativeModel::GetGlobalTopicDistribution() const {
  return global_distribution_;
}

void LDAAccumulativeModel::AppendAsString(map<string, int>* word_index_map,
                                          std::ostream& out) const {
  vector<string> index_word_map(word_index_map->size());
  for (map<string, int>::const_iterator iter = word_index_map->begin();
       iter != word_index_map->end(); ++iter) {
    index_word_map[iter->second] = iter->first;
  }
  for (int i = 0; i < topic_distributions_.size(); ++i) {
    out << index_word_map[i] << "\t";
    for (int topic = 0; topic < num_topics(); ++topic) {
      out << topic_distributions_[i][topic]
          << ((topic < num_topics() - 1) ? " " : "\n");
    }
  }
}

int LDAAccumulativeModel::partition (int *arr, int topic, int low, int high)
{
    int pivot = topic_distributions_[arr[high]][topic];    // pivot
    int i = (low - 1);  // Index of smaller element
    int tmp;
    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (topic_distributions_[arr[j]][topic] >= pivot)
        {
            i++;    // increment index of smaller element
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }
    }
    tmp = arr[i+1];
    arr[i+1] = arr[high];
    arr[high] = tmp;
    return (i + 1);
}

void LDAAccumulativeModel::sortWords(int * sortedIdx, int topic, int low, int high) {

  if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(sortedIdx, topic, low, high);

        // Separately sort elements before
        // partition and after partition
        sortWords(sortedIdx, topic, low, pi - 1);
        sortWords(sortedIdx, topic, pi + 1, high);
    }

}

string LDAAccumulativeModel::GetWordsPerTopic(map<string, int>* word_index_map, int topic, int nWords, string delimiter)  {
  vector<string> index_word_map(word_index_map->size());
  
  for (map<string, int>::const_iterator iter = word_index_map->begin();
       iter != word_index_map->end(); ++iter) {
    index_word_map[iter->second] = iter->first;
  }

  int totWords = topic_distributions_.size();

  int* sortedIdx = new int[totWords];

  for(int i=0; i<totWords; i++)
    sortedIdx[i] = i;

  sortWords(sortedIdx, topic, 0, totWords-1);

  string bestwords = "";
  for (int i = 0; i < nWords; ++i) {
    bestwords+= index_word_map[sortedIdx[i]] + delimiter;
  }

  return bestwords;
}

}  // namespace learning_lda
