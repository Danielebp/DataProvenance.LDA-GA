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

const double FITNESS_THRESHHOLD = 0.75;
const int MAX_TOPICS = 20;
const int MAX_ITERATIONS = 1000;

ResultStatistics geneticLogic(int population, int numberOfDocuments);
void sortInitialPopulation(PopulationConfig* mInitialPopulation, int size);

#endif
