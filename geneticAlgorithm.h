#ifndef __geneticAlgorithm_h__
#define __geneticAlgorithm_h__

#include <iostream>
#include <sstream>
#include <exception>
#include <unordered_map>
#include <float.h>
#include <math.h>

#include "utils.h"
#include "populationConfig.h"
#include "resultStatistics.h"
#include "parallelizables.h"
#include "TopicModelling.h"
#include "cluster.h"

using namespace std;

const int MAX_TOPICS = 20;
const int MAX_ITERATIONS = 1000;

ResultStatistics geneticLogic(int population, int numberOfDocuments, double fitnessThreshold);
void sortInitialPopulation(PopulationConfig* mInitialPopulation, int size);

#endif
