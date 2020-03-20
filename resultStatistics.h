#ifndef __resultStatistics_h__
#define __resultStatistics_h__

#include "populationConfig.h"

class ResultStatistics {
public:
	PopulationConfig cfg;
	double precision_percentage;
	double recall_percentage;
	long execution_milliseconds;

    long GA_count;
	long LDA_count;
	long LDA_time;

	string to_string(string head) ;

	string time_to_str(long milliseconds, unsigned long string_min_length) ;

	void OnLDAFinish(PopulationConfig cfg) ;
};

#endif
