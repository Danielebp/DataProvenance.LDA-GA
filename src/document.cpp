#include <iostream>
#include "document.h"
using namespace std;

Document::Document(string name, string keyWords) {
	this->name = name;
	this->keyWords = keyWords;
}
Document::Document() {
	// TODO Auto-generated constructor stub
	this->name = "";
	this->keyWords = "";
}

string Document::getName() {
	return name;
}
void Document::setName(string name) {
	this->name = name;
}
string Document::getKeyWords() {
	return keyWords;
}
void Document::setKeyWords(string keyWords) {
	this->keyWords = keyWords;
}
