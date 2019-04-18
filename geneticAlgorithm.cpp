
#include "geneticAlgorithm.h"

ResultStatistics geneticLogic(int population, int numberOfDocuments, double fitnessThreshold) {
    ResultStatistics result;

    cout<<"Starting GA"<<endl;
    cout<<"###########################"<<endl;

    int *initialPopulation = new int[population*2];
    for (int i = 0; i<population*2; i++){
        initialPopulation[i]   = (int) floor(getRandomFloat()*MAX_TOPICS + 2); // needs at least 2 topics
        initialPopulation[++i] = (int) floor(getRandomFloat()*MAX_ITERATIONS + 1); // needs at least 1 iteration
    }

    double * fitnessValues = new double[population];
    string * filesToDelete = new string[population];
    int GACounter = 0;
    int LDACounter = 0;

    clock_t t;
    clock_t total_t = clock();

    bool maxFitnessFound = false;
    bool checkLowThreshold;
    long LDATotTime = 0;


    while (!maxFitnessFound){
        GACounter ++;
        // runs population

        checkLowThreshold = true;
        for (int i = 0; i<population*2; i+=2){
            LDACounter ++;
            clock_t exTm = clock();

            t = clock();

            TopicModelling tm(initialPopulation[i], initialPopulation[i+1], numberOfDocuments);
            PopulationConfig popCfg;
            popCfg.number_of_topics = initialPopulation[i];
    		popCfg.number_of_iterations = initialPopulation[i+1];

            filesToDelete[i/2] = "__"+to_string(i/2)+"__"+to_string(initialPopulation[i])+"x"+to_string(initialPopulation[i+1]);
            tm.LDA(filesToDelete[i/2]);
            t = clock() - t;

            popCfg.LDA_execution_milliseconds = ((float)t)/(CLOCKS_PER_SEC/1000);
            LDATotTime += popCfg.LDA_execution_milliseconds;
            //number of topics - the first value
            int numberOfTopics = initialPopulation[i];

            // ********************** BREAKPOINT *********************************
            // TODO: still trying to understand it from here
            // how to calculate clusterCentroids, max/minDistance, etc...

            multimap <int, int> clusterMap;
            int mainTopic;
            int topicLess = 0;
            for(int d = 0; d < numberOfDocuments; d++){
                mainTopic = tm.getMainTopic(d);
                if(mainTopic>=0)clusterMap.insert(pair<int, int> (mainTopic, d));
                else{
                    topicLess++;
                    cout<<tm.getDocNameByNumber(d)<<endl;
                }
            }
            cout<<"Documents without topic: "<<topicLess<<endl;

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
                if(max(minDistanceOutsideCluster[m],maxDistanceInsideCluster[m]) <= 0)
                    silhouetteCoefficient[m] = 0;
                else
                    silhouetteCoefficient[m] = (minDistanceOutsideCluster[m] - maxDistanceInsideCluster[m]) / max(minDistanceOutsideCluster[m],maxDistanceInsideCluster[m]);
            }

            //find the average of the Silhouette coefficient of all the documents - fitness criteria
            double total = 0;
            for(int m = 0 ; m < (numberOfDocuments); m++ ) {
                total += silhouetteCoefficient[m];
            }
            fitnessValues[i/2] = total / (numberOfDocuments - topicLess);
            popCfg.fitness_value = fitnessValues[i/2];
            if(popCfg.fitness_value > fitnessThreshold) {
                tm.WriteFiles();
            }


            // ###########################################################
            // TODO: finish this par
            // precision and recalls
            int topicsWithDocs = clusterMap.size();
            float precision_total = 0;
            float recall_total = 0;
            int validTopics = 0;
    		if (topicsWithDocs <= 0) {
    			precision_total = 0;
                recall_total = 0;
                exTm = clock() - exTm;
    		}
            else {
                string line;
                float* precision = new float[topicsWithDocs];
                float* recall = new float[topicsWithDocs];
                unordered_map<string,string> truthData;
                ifstream myfile ("truthfile.txt");
                int stringCheck;

                while ( getline (myfile,line) ) {
                    stringCheck = line.find("#");
                    string split = line.substr(0, stringCheck);
                    truthData[line.substr(0, stringCheck)] = line.substr(stringCheck+2);
                }

        		for (int topicIdx = 0; topicIdx < topicsWithDocs; topicIdx++) {
        			precision[topicIdx] = 0;
        			recall[topicIdx] = 0;

                    int numOfDocumentsForTopic = clusterMap.count(topicIdx);

                    // topic does not have any document
                    if(numOfDocumentsForTopic <= 0) continue;

                    // only doing the first?
                    int counter = numOfDocumentsForTopic;
                    string currentArticle;

                    int sourceCounter = 0;
                    int articleCounter = 0;

                    for(multimap <int, int> :: iterator itr = clusterMap.find(topicIdx); counter>0; itr++, counter--){
                        currentArticle = tm.getDocNameByNumber(itr->second);
                        stringCheck = currentArticle.find("$AAA$");
                        if (stringCheck > 0){
                            sourceCounter = 0;
                            articleCounter = 0;
                            int precise_count = 0;
                            int recall_count = 0;
                            int counter2 = numOfDocumentsForTopic;
                            for(multimap <int, int> :: iterator documentPerTopicItr = clusterMap.find(topicIdx); counter2>0; documentPerTopicItr++, counter2--){
                                string documentPerTopic = tm.getDocNameByNumber(documentPerTopicItr->second);
                                if (currentArticle.compare(documentPerTopic)==0) continue;
                                stringCheck = documentPerTopic.find("$AAA$");
                                if (stringCheck > 0){
                                    articleCounter++;
                                    continue;
                                }
                                sourceCounter++;

                                stringstream s(truthData[currentArticle]);
                                string relatedDocument;

                                while (s >> relatedDocument){
                                    if(relatedDocument.compare(documentPerTopic)==0) {
                                        precise_count++;
                                    }
                                }
                            }

                            if(sourceCounter <= 0) {
                                cout<<"Topic["<<topicIdx<<"] does not have sources"<<endl;
                                precision[topicIdx] = 0;
                                recall[topicIdx] = 0;
                                continue;
                            }

                            precision[topicIdx] += (float) precise_count / (float) sourceCounter;

                            int numberOfRelatedDocs = 0;
                            stringstream s(truthData[currentArticle]);
                            string relatedDocument;
                            while (s >> relatedDocument){
                                if (currentArticle.compare(relatedDocument)==0) continue;
                                numberOfRelatedDocs++;
                                counter2 = numOfDocumentsForTopic;
                                for(multimap <int, int> :: iterator documentPerTopicItr = clusterMap.find(topicIdx); counter2>0; documentPerTopicItr++, counter2--){
                                    if(relatedDocument.compare(tm.getDocNameByNumber(documentPerTopicItr->second)) == 0) {
                                        recall_count++;
                                    }
                                }
                            }
                            if(numberOfRelatedDocs <= 0){
                                recall[topicIdx] += 1.0f/(articleCounter+1);
                            }
                            else {
                                recall[topicIdx] += (float) recall_count / (float) numberOfRelatedDocs;
                            }
                        }
                    }

                    if(precision[topicIdx] > 0 && recall[topicIdx] > 0) validTopics++;

        			precision_total  += precision[topicIdx];
        			if(articleCounter > 0) recall_total     += recall[topicIdx]/articleCounter;
                }
                exTm = clock() - exTm;
            }

            result.cfg = popCfg;
            result.execution_milliseconds = ((float)exTm)/(CLOCKS_PER_SEC/1000);
            result.precision_percentage = (float) (precision_total / validTopics) * 100;
            result.recall_percentage = (float) (recall_total / validTopics) * 100;
            cout<<result.to_string("")<<endl;

        }


        t = clock();

        //ranking and ordering the chromosomes based on the fitness function.
        //no sorting code found?(by Xiaolin)
        //We need only the top 1/3rd of the chromosomes with high fitness values - Silhouette coefficient
        int* newPopulation = new int[population*2];
        int spanSize = population/3;
        //copy only the top 1/3rd of the chromosomes to the new population
        for(int i = 0 ; i < 2*spanSize ; i+=2) {
            double maxFitness = INT_MIN;
            int maxFitnessChromosome = -1;
            for(int j = 0 ; j < population*2 ; j+=2) {
                if(fitnessValues[j/2] > maxFitness) {
                    maxFitness = fitnessValues[j/2];

                    // when maxFitness satisfies the requirement, stop running GA
                    if(maxFitness > fitnessThreshold) {


                        //run the function again to get the words in each topic
                        cout<<"the best distribution is "<<initialPopulation[j]<<" topics and "<<initialPopulation[j+1]<<" iterations and fitness is "<<maxFitness<<endl;
                        maxFitnessFound = true;
                        break;
                    }
                    maxFitnessChromosome = j;
                }
            }

            if(maxFitnessFound) {
                break;
            }

            if(checkLowThreshold) {
                // if best is this low, restart from zero
                if(maxFitness < 0) {
                    for (int q = 0; q<spanSize*2; q++){
                        newPopulation[q]   = (int) floor(getRandomFloat()*MAX_TOPICS + 2); // needs at least 2 topics
                        newPopulation[++q] = (int) floor(getRandomFloat()*MAX_ITERATIONS + 1); // needs at least 1 iteration
                    }
                    break;
                }
                checkLowThreshold = false;
            }

            //copy the chromosome with high fitness to the next generation
            newPopulation[i] = initialPopulation[maxFitnessChromosome];
            newPopulation[i+1] = initialPopulation[maxFitnessChromosome+1];
            fitnessValues[maxFitnessChromosome/2] = INT_MIN;
        }

        if(maxFitnessFound) {
            break;
        }


        //perform crossover - to fill the rest of the 2/3rd of the initial Population
        for(int i = 0 ; i < 2*spanSize  ; i+=2 ) {
            newPopulation[(2*spanSize)+i] = newPopulation[i];
            newPopulation[(2*spanSize)+i+1]   = (int) floor(getRandomFloat()*MAX_ITERATIONS + 1);

            newPopulation[(4*spanSize)+i]  = (int) floor(getRandomFloat()*MAX_TOPICS + 2);
            newPopulation[(4*spanSize)+i+1]= newPopulation[i+1];
        }

        //substitute the initial population with the new population and continue
        delete[] initialPopulation;
        initialPopulation = newPopulation;

        t = clock() - t;
        cout << "GA took " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms"<<endl;
    }

    total_t = clock() - total_t;

    cout<<endl<<"###########################################"<<endl;
    cout<<"Total LDA-GA time was: " << ((float)total_t)/(CLOCKS_PER_SEC/1000) << "ms" << endl;
    cout<<"Number of GA iterations: " << GACounter<<endl;
    cout<<"Number of LDA calls: " << LDACounter<<endl;
    cout<<"Average LDA time: " << LDATotTime/LDACounter << "ms" << endl;
    cout<<"###########################################"<<endl;


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
