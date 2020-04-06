#include "logger.h"


LogLevel logLevel;
ofstream logFile;

Logger::Logger(string filename, LogLevel level, LogLevel fileLevel){
    size_t lastindex = filename.find_last_of(".");
    string name = filename.substr(0, lastindex);
    string ext = filename.substr(lastindex);
    string timestamp = getTimestamp();

    filename = name + "_" + timestamp + ext;
    logFile.open (filename);
    logLevel = level;
    logFileLevel = fileLevel;
}

bool Logger::init(string filename, LogLevel level, LogLevel fileLevel){
    size_t lastindex = filename.find_last_of(".");
    string name = filename.substr(0, lastindex);
    string ext = filename.substr(lastindex);
    string timestamp = getTimestamp();

    filename = name +"_"+ timestamp + ext;
    logFile.open (filename);
    logLevel = level;
    logFileLevel = fileLevel;
    return true;
}

Logger::~Logger(){
    logFile.close();
}


string getTime(){
    auto givemetime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string t( ctime( &givemetime ) );
    return t.substr( 0, t.length() -1  );
}

bool Logger::log(LogLevel level, string message){
    if(level<=logLevel){
       cout << (level==error ? RED : (level==status ? GREEN : (level==info ? CYAN : WHITE))) << getLogLabel(level)<< "\tmessage at "<<getTime()<<": "<<message<<RESET<<endl;
    }
    if(level<=logFileLevel){
       logFile << getLogLabel(level)<< "\tmessage at "<<getTime()<<": "<<message<<endl;
    }

	return true;
}
