

#ifndef _STRTOKENIZER_H
#define _STRTOKENIZER_H

#include <string>
#include <vector>

using namespace std;

class strtokenizer {
protected:
    vector<string> tokens;
    unsigned int idx;

public:
    strtokenizer(string str, string seperators = " ");    
    
    void parse(string str, string seperators);
    
    int count_tokens();
    string next_token();   
    void start_scan();

    string token(unsigned int i);
};

#endif

