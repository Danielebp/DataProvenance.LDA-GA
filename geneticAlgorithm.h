#ifndef __geneticAlgorithm_h__
#define __geneticAlgorithm_h__

#include <iostream>
#include <sstream>
#include <exception>
#include <float.h>
#include <math.h>

#include "utils.h"
#include "populationConfig.h"
#include "resultStatistics.h"
#include "parallelizables.h"
#include "TopicModelling.h"

using namespace std;

const double FITNESS_THRESHHOLD = 0.35;

ResultStatistics geneticLogic(int population, int numberOfDocuments);
void sortInitialPopulation(PopulationConfig* mInitialPopulation, int size);

#endif
