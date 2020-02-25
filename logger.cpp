#include "logger.h"


LogLevel logLevel;
ofstream logFile;

Logger::Logger(string filename, LogLevel level){
    logFile.open (filename);
    logLevel = level;
}

bool Logger::init(string filename, LogLevel level){
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
       cout    << getLabel(level)<< "\tmessage at "<<getTime()<<": "<<message<<endl;
       logFile << getLabel(level)<< "\tmessage at "<<getTime()<<": "<<message<<endl;
    }
}
