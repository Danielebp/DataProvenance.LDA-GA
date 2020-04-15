#include "wordFilter.h"
using namespace std;


WordFilter::WordFilter(string filename) {
  string line;
  ifstream myfile (filename);

  while ( getline (myfile,line) )
  {
      line = trim(line);
      stopWords[line] = 0;
  }
  myfile.close();
}

string WordFilter::removeLineMarkers (string token) {
    if(token[0] == '\r')
        token = token.substr(1);

    if(token[token.length()-1] == '\r')
        token = token.substr(0, token.length()-1);

    return token;
}

bool WordFilter::isEmpty(string s) {
    if(s.empty()) return true;
    if(s.length()<1)    return true;
    if(s == "" || s == " ") return true;

    strtokenizer strtok(s, " \t\r\n");
    if(strtok.count_tokens() == 0) return true;
    return false;
}

bool WordFilter::isNumber(string s) {
    bool isNumb = false;
    if(isDigit(s[0])) {
        isNumb = true;
        for (unsigned int i=1; i < s.length(); i++) {
            if(!isDigit(s[i])) {
                isNumb = false;
                break;
            }
        }
    }

    return isNumb;
}

bool WordFilter::isDigit(char c) {
    for(char digit = '0'; digit <= '9'; digit++)
        if (c == digit) return true;

    if (c == '.' || c == ',') return true;

    return false;
}

string WordFilter::standardizeToLower(string s) {
    int i=0;
    string lower = "";
    while (s[i]) {
        if((s[i]>='a' && s[i]<='z')||(s[i]>='A' && s[i]<='Z')||(s[i]=='_'))
            lower += tolower(s[i]);

        i++;
    }
    return lower;
}
