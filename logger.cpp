#include "logger.h"


LogLevel logLevel;
ofstream logFile;

Logger::Logger(string filename, LogLevel level){
    size_t lastindex = filename.find_last_of("."); 
    string name = filename.substr(0, lastindex); 
    string ext = filename.substr(lastindex);
    string timestamp = getTimestamp();

    filename = name + "_" + timestamp + ext;
    logFile.open (filename);
    logLevel = level;
}

bool Logger::init(string filename, LogLevel level){
    size_t lastindex = filename.find_last_of(".");
    string name = filename.substr(0, lastindex);
    string ext = filename.substr(lastindex);
    string timestamp = getTimestamp();

    filename = name +"_"+ timestamp + ext;
    logFile.open (filename);
    logLevel = level;
    return true;
}

Logger::~Logger(){
    logFile.close();
}

string getLabel(LogLevel level){
    switch(level) {
       case debug:
          return "DEBUG";
       case info:
          return "INFO";
       case error:
          return "ERROR";
       default :
          "UNLABELED";
    }
}

string getTime(){
    auto givemetime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string t( ctime( &givemetime ) );
    return t.substr( 0, t.length() -1  );
}

bool Logger::log(LogLevel level, string message){
    if(level<=logLevel){
       cout << (level==error ? RED : (level==info ? GREEN : WHITE)) << getLabel(level)<< "\tmessage at "<<getTime()<<": "<<message<<RESET<<endl;
       logFile << getLabel(level)<< "\tmessage at "<<getTime()<<": "<<message<<endl;
    }
}
