#include "scanner.h"

Scanner::Scanner () {
    ss<<"";
}

Scanner::Scanner (string filename) {
    string line;
    myfile.open(filename);
    getline (myfile, line, '\n');
    ss<<line;
}

void Scanner::open (string filename) {
    string line;
    myfile.open(filename);
    getline (myfile, line, '\n');
    ss<<line;
}

bool Scanner::nextLine() {
    string line;
    if(getline (myfile, line, '\n')) {
        ss.str(line);
        return true;
    }
    ss<<"";
    return false;
}

bool Scanner::hasNextLine() {
    int c = myfile.peek();  // peek character
    return (c != EOF);
}

void Scanner::close() {
      myfile.close();
      ss.str("");
}

int Scanner::nextInt(){
    string temp;
    int found;

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
    double dfound;
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

string Scanner::nextWord(){
    string word;
    while (!ss.eof()) {

        /* extracting word by word from stream */
        ss >> word;

        /* Checking the given word is integer or not */
        if (!word.empty())
            return word;

        /* To save from space at the end of string */
        word = "";
    }
    return "";
}
