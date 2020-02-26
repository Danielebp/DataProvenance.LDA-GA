#include <iostream>
#include <time.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <dirent.h>
#include <vector>
#include "document.h"
#include "utils.h"
#include "config.h"
#include "wordFilter.h"

using namespace std;

unordered_map<string, Document> preProcess(ConfigOptions* cfg);
unordered_map<string, Document> loadPreProcessed(ConfigOptions* cfg);
