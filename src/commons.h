#ifndef __commons_h__
#define __commons_h__

#include <iostream>
#include "utils.h"

#define UNLIKELY(x) __builtin_expect( (bool)(x), 0)

enum LogLevel { error, status, info, debug };
enum RunType { metric, train };
enum PerfType { cpu, cuda };
enum LDALibrary { plda, gibbs, glda, blda, wlda };

string getLogLabel(LogLevel level);
string getLibraryLabel(LDALibrary library);

#endif
