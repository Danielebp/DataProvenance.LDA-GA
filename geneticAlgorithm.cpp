
#include "geneticAlgorithm.h"

ResultStatistics geneticLogic(int population, int numberOfDocuments) {
    ResultStatistics result;

    int *initialPopulation = new int[population*2];
    for (int i = 0; i<population*2; i++){
        initialPopulation[i]   = (int) floor(getRandomFloat()*12 + 3);
        initialPopulation[++i] = (int) floor(getRandomFloat()*1000 + 1);
    }

    clock_t t = clock();
    TopicModelling tm;

    for (int i = 0; i<population*2; i+=2){


        tm.LDA(initialPopulation[i], initialPopulation[i+1], true);

        //number of topics - the first value
        int numberOfTopics = initialPopulation[i];

        // ********************** BREAKPOINT *********************************
        // TODO: still trying to understand it from here
        // how to calculate clusterCentroids, max/minDistance, etc...

        //getting the centroid of each cluster by calculating the average of their cluster distribution
        double* clusterCentroids = new double[numberOfTopics];
        for(int k = 0; k<numberOfTopics; k++){
            for(int j = 0; j<numberOfDocuments; j++){
                clusterCentroids[k] += tm.getDistribution(k, j);
            }
            clusterCentroids[k] /= numberOfDocuments;
        }

        //finding the distance of each documents in each cluster finding max distance from other documents in the same cluster
        double[] maxDistanceInsideCluster = new double[numberOfDocuments];
        double[] minDistanceOutsideCluster = new double[numberOfDocuments];

        for(int docNo = 0; docNo<numberOfDocuments; docNo++){
            maxDistanceInsideCluster[docNo] = 0;
            minDistanceOutsideCluster[docNo] = INT_MAX;

            //for each of the documents find the maxDistance from other cluster members
            for(int otherDocNo = 0 ; otherDocNo < numberOfDocuments ; otherDocNo++ ) {
                if(otherDocNo == docNo){
                    continue;
                }

                //finding euclidean distance between the two points/docuemnts
                double distance = 0;
                for(int h = 0 ; h < numberOfTopics ; h++) {
                    distance += pow((tm.getDistribution(h, otherDocNo) - tm.getDistribution(h, docNo), 2);
                }
                distance = sqrt(distance);
                if (distance > maxDistanceInsideCluster[docNo]){
                    maxDistanceInsideCluster[docNo] = distance;
                }
            }

            //find the documents min distance from the centroid of other clusters
            double distance = 0;
            for(int h = 0 ; h < numberOfTopics ; h++) {
                distance +=  pow((clusterCentroids[h] - tm.getDistribution(h, docNo)), 2);
            }
            distance = sqrt(distance);
            if (distance < minDistanceOutsideCluster[docNo]){
                minDistanceOutsideCluster[docNo] = distance;
        }


        //calculate the Silhouette coefficient for each document
        double[] silhouetteCoefficient = new double[numberOfDocuments];
        for(int m = 0 ; m < (numberOfDocuments); m++ ) {
            silhouetteCoefficient[m] = (minDistanceOutsideCluster[m] - maxDistanceInsideCluster[m]) / max(minDistanceOutsideCluster[m],maxDistanceInsideCluster[m]);
        }


        //find the average of the Silhouette coefficient of all the documents - fitness criteria
        double total = 0;
        for(int m = 0 ; m < (numberOfDocuments); m++ ) {
            total += silhouetteCoefficient[m];
        }
        fitnessValues[i/2] = total / (numberOfDocuments - 1);
    }


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
