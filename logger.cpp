#include "logger.h"


LogLevel logLevel;
ofstream logFile;

Logger::Logger(string filename, LogLevel level){
    logFile.open (filename);
    logLevel = level;
}

Logger::~Logger(){
    myfile.close();
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
          "UNLABELED"
    }
}

bool Logger::log(LogLevel level, string message){
    if(level<=logLevel)myfile << level<< " message at " ctime(&timenow)<<": "<<message<<endl;

}
