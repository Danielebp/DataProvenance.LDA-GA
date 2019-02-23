#ifndef __document_h__
#define __document_h__

#include <iostream>
using namespace std;

class Document {

public:
	string name;
	string keyWords;

	Document();
	Document(string name, string keyWords);

	string getName() ;
	void setName(string name) ;
	string getKeyWords() ;
	void setKeyWords(string keyWords) ;

};

#endif
