#ifndef __utils_h__
#define __utils_h__

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
using namespace std;

string trim(string str);
string getFileName (const string& filepath);
float getRandomFloat();
vector<string> split(string& s, char delimiter);
string getTimestamp();

#endif
