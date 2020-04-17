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
    if(myfile)
        myfile.open(filename);

    getline (myfile, line, '\n');
    ss.str(line);
}

bool Scanner::nextLine() {
    string line;
    ss.str(std::string());
    ss.clear();
    if(getline (myfile, line, '\n')) {
        ss.str(line);
        return true;
    }
    ss.str("");
    return false;
}


bool Scanner::nextLineClean() {
    string line;
    unsigned int pos;
    ss.str(std::string());
    ss.clear();
    if(getline (myfile, line, '\n')) {
        pos = line.find("ERROR");
        while(pos!=std::string::npos && pos<line.size()){
            getline (myfile, line, '\n');
            pos = line.find("ERROR");
        }
        pos = line.find("STATUS");
        if(pos!=std::string::npos  && pos<line.size())line.replace(pos, 43, " ");
        pos = line.find(":");
        while(pos!=std::string::npos && pos<line.size()){
            line.replace(pos, 1, " ");
            pos = line.find(":");
        }
        pos = line.find("ms");
        while(pos!=std::string::npos && pos<line.size()){
            line.replace(pos, 2, " ");
            pos = line.find("ms");
        }
        pos = line.find("(");
	while(pos!=std::string::npos && pos<line.size()){
            int pos2 = line.find(")");
            line.replace(pos, pos2-pos, " ");
            pos = line.find("(");
        }

        pos = line.find("calls");
        if(pos!=std::string::npos && pos<line.size())line.replace(pos, 5, "calls ");

        ss.str(line);
        return true;
    }
    ss.str("");
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
        if (stringstream(temp) >> found){
            return found;
        }

        /* To save from space at the end of string */
        temp = "";
    }
    return 0;
}

double Scanner::nextDouble(){
    string temp;
    double dfound;
    while (!ss.eof()) {

        /* extracting word by word from stream */
        ss >> temp;

        /* Checking the given word is integer or not */
        if (stringstream(temp) >> dfound){
            return dfound;
        }

        /* To save from space at the end of string */
        temp = "";
    }
    return 0;
}

string Scanner::nextWord(){
    string word;
    while (!ss.eof()) {

        /* extracting word by word from stream */
        ss >> word;

        /* Checking the given word is integer or not */
        if (!word.empty()){
            return word;
        }

        /* To save from space at the end of string */
        word = "";
    }
    return "";
}
