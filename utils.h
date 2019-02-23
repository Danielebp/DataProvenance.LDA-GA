#ifndef __utils_h__
#define __utils_h__

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

string trim(string str);
bool isDigit(char c);
bool isNumber(string s);
string standardizeToLower(string s);
string getFileName (const string& filepath);
float getRandomFloat();

#endif
