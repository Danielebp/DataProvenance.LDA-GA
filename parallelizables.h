#ifndef __PARALLELIZABLES_H__
#define __PARALLELIZABLES_H__

#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <limits.h>
#include <map>
#include <iterator>

#include "populationConfig.h"
#include "scanner.h"

using namespace std;

void parallelizableStart(int thread_index, PopulationConfig population_cfg, int population_index, int numberOfDocuments, bool original_version) ;

#endif
