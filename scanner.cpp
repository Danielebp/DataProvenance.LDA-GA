#include "scanner.h"

Scanner::Scanner (string str) {
  ss << str;
}

int Scanner::nextInt(){
    string temp;
    while (!ss.eof()) {

        /* extracting word by word from stream */
        ss >> temp;

        /* Checking the given word is integer or not */
        if (stringstream(temp) >> found)
            return found;

        /* To save from space at the end of string */
        temp = "";
    }
    return NULL;
}

double Scanner::nextDouble(){
    string temp;
    while (!ss.eof()) {

        /* extracting word by word from stream */
        ss >> temp;

        /* Checking the given word is integer or not */
        if (stringstream(temp) >> dfound)
            return dfound;

        /* To save from space at the end of string */
        temp = "";
    }
    return NULL;
}
