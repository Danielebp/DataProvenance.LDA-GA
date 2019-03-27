
#include "geneticAlgorithm.h"

ResultStatistics geneticLogic(int population, int numberOfDocuments) {
    ResultStatistics result;

    cout<<"Starting GA"<<endl;
    cout<<"###########################"<<endl;

    int *initialPopulation = new int[population*2];
    for (int i = 0; i<population*2; i++){
        initialPopulation[i]   = (int) floor(getRandomFloat()*MAX_TOPICS + 2); // needs at least 2 topics
        initialPopulation[++i] = (int) floor(getRandomFloat()*MAX_ITERATIONS + 1); // needs at least 1 iteration
    }

    double * fitnessValues = new double[population];

    clock_t t = clock();

    for (int i = 0; i<population*2; i+=2){

        TopicModelling tm(initialPopulation[i], initialPopulation[i+1], numberOfDocuments);

        tm.LDA("__"+to_string(i/2)+"__"+to_string(initialPopulation[i])+"x"+to_string(initialPopulation[i+1]));

        //number of topics - the first value
        int numberOfTopics = initialPopulation[i];

        // ********************** BREAKPOINT *********************************
        // TODO: still trying to understand it from here
        // how to calculate clusterCentroids, max/minDistance, etc...

        multimap <int, int> clusterMap;
        for(int d = 0; d < numberOfDocuments; d++){
            clusterMap.insert(pair<int, int> (tm.getMainTopic(d), d));
        }

        //getting the centroid of each cluster by calculating the average of their cluster distribution
        double* clusterCentroids = new double[numberOfTopics*numberOfTopics];
        for(int k = 0; k<numberOfTopics; k++){
            int docsOfK = clusterMap.count(k);
            // topic does not have any document
            if(docsOfK <= 0) continue;

            int counter = docsOfK;
            for(multimap <int, int> :: iterator itr = clusterMap.find(k); counter>0; itr++, counter--){
                for (int t = 0; t<numberOfTopics; t++) {
                    clusterCentroids[(k*numberOfTopics)+t] += tm.getDistribution(t, itr->second);
                }
            }
            for (int t = 0; t<numberOfTopics; t++) {
                clusterCentroids[(k*numberOfTopics)+t] /= docsOfK;
            }
        }

        double* maxDistanceInsideCluster = new double[numberOfDocuments];
        double* minDistanceOutsideCluster = new double[numberOfDocuments];

        // finding the distance of each documents in each cluster
        // finding max distance from other documents in the same cluster
        for(int k = 0; k<numberOfTopics; k++){
            int docsOfK = clusterMap.count(k);
            // topic does not have any document
            if(docsOfK <= 0) continue;

            int counter = docsOfK;
            for(multimap <int, int> :: iterator itr = clusterMap.find(k); counter>0; itr++, counter--){
                int docNo = itr->second;
                maxDistanceInsideCluster[docNo] = 0;

                int counter2 = docsOfK;
                for(multimap <int, int> :: iterator itr2 = clusterMap.find(k); counter2>0; itr2++, counter2--){
                    int otherDocNo = itr2->second;
                    if(otherDocNo == docNo){
                        continue;
                    }

                    //finding euclidean distance between the two points/docuemnts
                    double distance = 0;
                    for(int h = 0 ; h < numberOfTopics ; h++) {
                        distance += pow((tm.getDistribution(h, otherDocNo) - tm.getDistribution(h, docNo)), 2);
                    }
                    distance = sqrt(distance);
                    if (distance > maxDistanceInsideCluster[docNo]){
                        maxDistanceInsideCluster[docNo] = distance;
                    }
                }
            }
        }

        //finding each documents minimum distance to the centroids of other clusters
        for(int k = 0; k<numberOfTopics; k++){
            int docsOfK = clusterMap.count(k);
            // topic does not have any document
            if(docsOfK <= 0) continue;

            int counter = docsOfK;
            for(multimap <int, int> :: iterator itr = clusterMap.find(k); counter>0; itr++, counter--){
                int docNo = itr->second;
                minDistanceOutsideCluster[docNo] = INT_MAX;
                for(int z = 0 ; z < numberOfTopics ; z++) {

                    //don't calculate the distance to the same cluster
                    if(z == k) {
                        continue;
                    }
                    double distance = 0;
                    for(int h = 0 ; h < numberOfTopics ; h++) {
                        distance += pow((clusterCentroids[(z*numberOfTopics)+h] - tm.getDistribution(h, docNo)), 2);
                    }
                    distance = sqrt(distance);
                    if (distance < minDistanceOutsideCluster[docNo]){
                        minDistanceOutsideCluster[docNo] = distance;
                    }
                }
            }
        }

        //calculate the Silhouette coefficient for each document
        double* silhouetteCoefficient = new double[numberOfDocuments];
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
    cout << "Fitness Levels: " << endl;
    for (int i = 0; i<population; i++) {
        cout <<"\t"<< fitnessValues[i] << endl;
    }

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
