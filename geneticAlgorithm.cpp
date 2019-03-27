
#include "geneticAlgorithm.h"

ResultStatistics geneticLogic(int population, int numberOfDocuments) {
    ResultStatistics result;

    // initialPopulation[i][0] = (int) Math.floor(Math.random()*12 + 3);
    // initialPopulation[i][1] = (int) Math.floor(Math.random()*1000 + 1);

    clock_t t = clock();

    // TO DO: perform GA
    TopicModelling tm;
    tm.LDA(15, 500, true);

    t = clock() - t;
    cout << "LDA took " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms"<<endl;

    return result;
}

void sortInitialPopulation(PopulationConfig* mInitialPopulation, int size) {
	if(mInitialPopulation==NULL || size <= 0)
	{
		return;
	}

	PopulationConfig* sortedPopulation = new PopulationConfig[size];
	for(int i=0, j=size; i<size && j>0; i++)
	{
		if(mInitialPopulation[i].fitness_value>0.0f) {
			sortedPopulation[i - (size - j)] = mInitialPopulation[i];
		}
		else {
			sortedPopulation[--j] = mInitialPopulation[i];
		}
	}
	mInitialPopulation = sortedPopulation;
}
