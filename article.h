#ifndef __article_h__
#define __article_h__

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "document.h"

using namespace std;

class Article : public Document{

public:

    //the article will have a unique set of keywords
	//this is used only in 2nd level filtering where the
	//words specific to an article is to be found
	unordered_set<string> uniqueKeyWordSet;

	//trying the hashtable technique for cleaning cluster approach 2
	unordered_map<string, int> uniqueKeyWords;
	int totalWordCount = 0;

    Article() : Document(){ }
    Article(string name, string keywords) : Document(name, keywords){ }
};

#endif
