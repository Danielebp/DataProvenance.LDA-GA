#ifndef __wordFilter_h__
#define __wordFilter_h__

#include <iostream>
#include <unordered_map>
#include <fstream>
#include "utils.h"
#include "LDA_Libraries/gldaCuda/src/strtokenizer.h"


using namespace std;

class WordFilter {
private:
  unordered_map<string,long> stopWords;

public:
  WordFilter(string filename);

  inline bool isStopWord(string s) { return stopWords.find(s) != stopWords.end(); }

  string removeLineMarkers (string token);

  bool isEmpty(string s);

  bool isNumber(string s);

  bool isDigit(char c);

  string standardizeToLower(string s);

};

#endif
