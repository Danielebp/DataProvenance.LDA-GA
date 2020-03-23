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
          return "UNLABELED";
    }
   return "";
}

string getLibraryLabel(LDALibrary library){
	switch(library) {
		case plda: 
			return "PLDA";
		case gibbslda:
			return "GibbsLDA++";
		case glda:
			return "GLDA";
	}
	return "";
}
