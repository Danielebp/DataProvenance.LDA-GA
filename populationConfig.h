#ifndef __populationConfig_h__
#define __populationConfig_h__

#include <math.h>
#include "utils.h"

class PopulationConfig
{
	public:
		static const long MIN_TOPIC_COUNT = 2;
		static const long MAX_TOPIC_COUNT = 15;
		static const long MIN_ITERATION_COUNT = 1;
		static const long MAX_ITERATION_COUNT = 1000;

		int number_of_topics;
		int number_of_iterations;
		double fitness_value;
		long LDA_execution_milliseconds;

		PopulationConfig() {}


		void copy(PopulationConfig rhs) ;

		void random();

		void random_topic();

		void random_iteration();

		string to_string();

		string to_string_all();
};

PopulationConfig* initArray(int count);

#endif
