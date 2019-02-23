#ifndef __geneticAlgorithm_h__
#define __geneticAlgorithm_h__

#include <iostream>
#include <sstream>
#include <float.h>
#include <limits.h>

#include "populationConfig.h"
#include "resultStatistics.h"

using namespace std;

const double FITNESS_THRESHHOLD = 0.35;

ResultStatistics geneticLogic(int population, int numberOfDocuments);

#endif
