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

#include "common.h"

char kSegmentFaultCauser[] = "Used to cause artificial segmentation fault";


TopicCountDistribution::TopicCountDistribution()
  : distribution_(NULL), size_(0) {}
TopicCountDistribution::TopicCountDistribution(int64* distribution, int size)
  : distribution_(distribution), size_(size) {}
void TopicCountDistribution::Reset(int64* distribution, int size) {
    distribution_ = distribution;
    size_ = size;
}
int TopicCountDistribution::size() const {
return size_;
}
void TopicCountDistribution::clear() {
memset(distribution_, 0, sizeof(*distribution_) * size_);
}

namespace learning_lda {

DocumentWordTopicsPB::DocumentWordTopicsPB() { wordtopics_start_index_.push_back(0); }
int DocumentWordTopicsPB::words_size() const { return words_.size(); }
int DocumentWordTopicsPB::wordtopics_count(int word_index) const {
    return wordtopics_start_index_[word_index + 1] - wordtopics_start_index_[word_index];
}
int DocumentWordTopicsPB::word_last_topic_index(int word_index) const {
    return wordtopics_start_index_[word_index + 1] - 1;
}
int DocumentWordTopicsPB::word(int word_index) const { return words_[word_index]; }
int32 DocumentWordTopicsPB::wordtopics(int index) const { return wordtopics_[index]; }
int32* DocumentWordTopicsPB::mutable_wordtopics(int index) { return &wordtopics_[index]; }

void DocumentWordTopicsPB::add_wordtopics(const string& word_s,
                  int word, const vector<int32>& topics) {
words_s_.push_back(word_s);
words_.push_back(word);
wordtopics_start_index_.pop_back();
wordtopics_start_index_.push_back(wordtopics_.size());
for (size_t i = 0; i < topics.size(); ++i) {
  wordtopics_.push_back(topics[i]);
}
wordtopics_start_index_.push_back(wordtopics_.size());
}
void DocumentWordTopicsPB::CopyFrom(const DocumentWordTopicsPB& instance) { *this = instance; }


bool IsValidProbDistribution(const TopicProbDistribution& dist) {
  const double kUnificationError = 0.00001;
  double sum_distribution = 0;
  for (int k = 0; k < dist.size(); ++k) {
    sum_distribution += dist[k];
  }
  return (sum_distribution - 1) * (sum_distribution - 1)
      <= kUnificationError;
}

int GetAccumulativeSample(const vector<double>& distribution) {
  double distribution_sum = 0.0;
  for (int i = 0; i < distribution.size(); ++i) {
    distribution_sum += distribution[i];
  }

  double choice = RandDouble() * distribution_sum;
  double sum_so_far = 0.0;
  for (int i = 0; i < distribution.size(); ++i) {
    sum_so_far += distribution[i];
    if (sum_so_far >= choice) {
      return i;
    }
  }

  //LOG(FATAL) << "Failed to choose element from distribution of size "
  //         << distribution.size() << " and sum " << distribution_sum;

  return -1;
}

std::ostream& operator << (std::ostream& out, vector<double>& v) {
  for (size_t i = 0; i < v.size(); ++i) {
    out << v[i] << " ";
  }
  return out;
}

}  // namespace learning_lda
