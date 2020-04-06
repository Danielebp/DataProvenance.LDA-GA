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
#include "config.h"

using namespace std;
class GeneticAlgorithm {
	int MAX_TOPICS;
	int MAX_ITERATIONS;

	bool calculateCentroids(double* clusterCentroids, multimap <int, int>* clusterMap, TopicModelling* tm, int numberOfTopics);
	bool getMaxDistancesInsideClusters(double* maxDistanceInsideCluster,
                                        multimap <int, int>* clusterMap,
                                        TopicModelling* tm,
                                        int numberOfTopics);
	bool getMinDistancesOutsideClusters(double* minDistanceOutsideCluster,
                                        multimap <int, int>* clusterMap,
                                        TopicModelling* tm,
                                        int numberOfTopics, ConfigOptions* cfg);
	double calculateFitness(TopicModelling* tm, int numberOfTopics, int numberOfDocuments, ConfigOptions* cfg) ;
	PopulationConfig* mutateToNewPopulation (PopulationConfig* population, ConfigOptions* cfg);
public:
        GeneticAlgorithm();

	ResultStatistics geneticLogic(int numberOfDocuments, ConfigOptions* cfg);
	void sortInitialPopulation(PopulationConfig* mInitialPopulation, int size);
};
#endif
