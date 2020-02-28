#include "commons.h"

string getLogLabel(LogLevel level){
    switch(level) {
       case debug:
          return "DEBUG";
       case status:
          return "STATUS";
       case info:
          return "INFO";
       case error:
          return "ERROR";
       default :
          "UNLABELED";
    }
}


