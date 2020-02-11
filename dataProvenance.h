#ifndef __dataProvenance_h__
#define __dataProvenance_h__

#include <iostream>
#include <sstream>
#include <exception>
#include <unordered_map>
#include <float.h>
#include <math.h>

#include "utils.h"
#include "resultStatistics.h"
#include "geneticAlgorithm.h"
#include "cluster.h"

using namespace std;

ResultStatistics reconstructProvenance(int populationSize, int numberOfDocuments, double fitnessThreshold, bool debug, bool progress) ;

#endif
