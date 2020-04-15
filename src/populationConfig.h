#ifndef __populationConfig_h__
#define __populationConfig_h__

#include <math.h>
#include "utils.h"

class PopulationConfig
{
	public:
		long MIN_TOPIC_COUNT = 2;
		long MAX_TOPIC_COUNT = 15;
		long MIN_ITERATION_COUNT = 1;
		long MAX_ITERATION_COUNT = 1000;

		int number_of_topics;
		int number_of_iterations;
		long seed;
		double fitness_value;
		long LDA_execution_milliseconds;

		PopulationConfig(long _seed) ;
                PopulationConfig() ;

        void set_max(int max_topic, int max_iteration);

		void copy(PopulationConfig rhs) ;

		void random();

		void random_topic();

		void random_iteration();

		string to_string();

		string to_string_all();
};

PopulationConfig* initArray(int count);
string to_string(PopulationConfig* population, int count);

#endif
