
#include "geneticAlgorithm.h"

bool calculateCentroids(double* clusterCentroids, multimap <int, int>* clusterMap, TopicModelling* tm, int numberOfTopics){
    for(int k = 0; k<numberOfTopics; k++){
        int docsOfK = clusterMap->count(k);
        // topic does not have any document
        if(docsOfK <= 0) continue;

        int counter = docsOfK;

        for(multimap <int, int> :: iterator itr = clusterMap->find(k); counter>0; itr++, counter--){
            for (int t = 0; t<numberOfTopics; t++) {
                clusterCentroids[(k*numberOfTopics)+t] += tm->getDistribution(t, itr->second);
            }
        }
        for (int t = 0; t<numberOfTopics; t++) {
            clusterCentroids[(k*numberOfTopics)+t] /= docsOfK;
        }
    }
    return true;
}

// finding the distance of each documents in each cluster
// finding max distance from other documents in the same cluster
bool getMaxDistancesInsideClusters(double* maxDistanceInsideCluster,
                                        multimap <int, int>* clusterMap,
                                        TopicModelling* tm,
                                        int numberOfTopics){

    for(int k = 0; k<numberOfTopics; k++){
        int docsOfK = clusterMap->count(k);
        // topic does not have any document
        if(docsOfK <= 0) continue;

        int counter = docsOfK;
        for(multimap <int, int> :: iterator itr = clusterMap->find(k); counter>0; itr++, counter--){
            int docNo = itr->second;
            maxDistanceInsideCluster[docNo] = 0;

            int counter2 = docsOfK;
            for(multimap <int, int> :: iterator itr2 = clusterMap->find(k); counter2>0; itr2++, counter2--){
                int otherDocNo = itr2->second;
                if(otherDocNo == docNo){
                    continue;
                }

                //finding euclidean distance between the two points/docuemnts
                double distance = 0;
                for(int h = 0 ; h < numberOfTopics ; h++) {
                    distance += pow((tm->getDistribution(h, otherDocNo) - tm->getDistribution(h, docNo)), 2);
                }
                distance = sqrt(distance);
                if (distance > maxDistanceInsideCluster[docNo]){
                    maxDistanceInsideCluster[docNo] = distance;
                }
            }
        }
    }

    return true;
}

//finding each documents minimum distance to the centroids of other clusters
bool getMinDistancesOutsideClusters(double* minDistanceOutsideCluster,
                                        multimap <int, int>* clusterMap,
                                        TopicModelling* tm,
                                        int numberOfTopics, ConfigOptions* cfg){
    // first calculates centroids
    double* clusterCentroids = new double[numberOfTopics*numberOfTopics];
    if(!calculateCentroids(clusterCentroids, clusterMap, tm, numberOfTopics))
        cfg->logger.log(error, "Error getting centroids");

    for(int k = 0; k<numberOfTopics; k++){
        int docsOfK = clusterMap->count(k);
        // topic does not have any document
        if(docsOfK <= 0) continue;

        int counter = docsOfK;
        for(multimap <int, int> :: iterator itr = clusterMap->find(k); counter>0; itr++, counter--){
            int docNo = itr->second;
            minDistanceOutsideCluster[docNo] = INT_MAX;
            for(int z = 0 ; z < numberOfTopics ; z++) {
                //don't calculate the distance to the same cluster
                if(z == k) {
                    continue;
                }
                double distance = 0;
                for(int h = 0 ; h < numberOfTopics ; h++) {
                    distance += pow((clusterCentroids[(z*numberOfTopics)+h] - tm->getDistribution(h, docNo)), 2);
                }
                distance = sqrt(distance);
                if (distance < minDistanceOutsideCluster[docNo]){
                    minDistanceOutsideCluster[docNo] = distance;
                }
            }
        }
    }
    return true;
}

double calculateFitness(TopicModelling* tm, int numberOfTopics, int numberOfDocuments, ConfigOptions* cfg) {
    stringstream ss;
    multimap <int, int> clusterMap;
    int mainTopic;
    int topicLess = 0;

    // cluster docs by main topic to calculate fitness
    for(int d = 0; d < numberOfDocuments; d++){
        mainTopic = tm->getMainTopic(d);
        if(mainTopic>=0)clusterMap.insert(pair<int, int> (mainTopic, d));
        else{
            topicLess++;
        }
    }
    ss<<"Documents without topic: "<<topicLess;
    cfg->logger.log(debug, ss.str());
    ss.str(std::string());
    ss.clear();;


    double* maxDistanceInsideCluster = new double[numberOfDocuments];
    if(!getMaxDistancesInsideClusters(maxDistanceInsideCluster, &clusterMap, tm, numberOfTopics))
        cfg->logger.log(error, "Error getting distances inside cluster");

    double* minDistanceOutsideCluster = new double[numberOfDocuments];
    if(!getMinDistancesOutsideClusters(minDistanceOutsideCluster, &clusterMap, tm, numberOfTopics, cfg))
        cfg->logger.log(error, "Error getting distances outside cluster");


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

    return (total / (numberOfDocuments - topicLess));
}

ResultStatistics geneticLogic(int numberOfDocuments, ConfigOptions* cfg) {

    stringstream ss;
    ResultStatistics result;
    PopulationConfig* population = new PopulationConfig[cfg->populationSize];

    cfg->logger.log(debug, "Starting GA");
    cfg->logger.log(debug, "###########################");

    // initialize population
    for (int i = 0; i<cfg->populationSize; i++){
        population[i].set_max(MAX_TOPICS, MAX_ITERATIONS); // define max of topics and iterations
        population[i].random(); // generate random number of topics and iterations
    }

    int GACounter = 0;
    int LDACounter = 0;
    long LDATotTime = 0;

    clock_t t;

    // add a limit of 1000 GA iterations, it should not run infinitly
    while (GACounter<1000){
        GACounter ++;
        ss<<"GA Attempt: "<<GACounter;
        cfg->logger.log(info, ss.str());
        ss.str(std::string());
        ss.clear();
        
        bool checkLowThreshold = true;

        // runs LDA for each pair on the population
        for (int i = 0; i<cfg->populationSize; i++){
            LDACounter ++;

            // creates temporary files for each LDA run
            string tempFileID = "__"+to_string(i)+"__"+to_string(population[i].number_of_topics)+"x"+to_string(population[i].number_of_topics);

            // creates TopicModelling obj and runs lda for pair i
            TopicModelling tm(population[i].number_of_topics, population[i].number_of_iterations, numberOfDocuments, cfg);
            long ldaTime = tm.LDA(tempFileID);

            // updates times
            population[i].LDA_execution_milliseconds = ldaTime;
            LDATotTime += population[i].LDA_execution_milliseconds;

            // calculates fitness value to determine wether to stop or try next pair
            population[i].fitness_value = calculateFitness(&tm, population[i].number_of_topics, numberOfDocuments, cfg);
            ss<<"LDA Attempt: "<<LDACounter<<" - Fitness: "<<population[i].fitness_value;
            cfg->logger.log(info, ss.str());
            ss.str(std::string());
            ss.clear();
            if(population[i].fitness_value >= cfg->fitnessThreshold) {
                cfg->logger.log(info, "Achieved fitness");
                // if fitness was achieved write dist files
                tm.WriteFiles();
                break;
            }

        }

        t = clock();

        //ranking and ordering the chromosomes based on the fitness function.
        //We need only the top 1/3rd of the chromosomes with high fitness values - Silhouette coefficient
        PopulationConfig* newPopulation = new PopulationConfig[cfg->populationSize];
        int spanSize = cfg->populationSize/3;
        //copy only the top 1/3rd of the chromosomes to the new population
        for(int i = 0 ; i < spanSize ; i++) {
            double maxFitness = INT_MIN;
            int maxFitnessChromosome = -1;
            for(int j = 0 ; j < cfg->populationSize ; j++) {
                if(population[j].fitness_value > maxFitness) {
                    maxFitness = population[j].fitness_value;

                    // when maxFitness satisfies the requirement, stop running GA
                    if(maxFitness >= cfg->fitnessThreshold) {
                        // TODO: running the LDA again, which is not such a great idea, see if it can be removed
                        cfg->logger.log(debug, "Re-run LDA");
                        TopicModelling tm(population[j].number_of_topics, population[j].number_of_iterations, numberOfDocuments, cfg);
                        tm.LDA("");
                        cfg->logger.log(debug, "Ran LDA");
                        tm.WriteFiles();
                        cfg->logger.log(debug, "Wrote files");

                        //run the function again to get the words in each topic
                        ss<<"the best distribution is "<<population[j].number_of_topics<<" topics and "<<population[j].number_of_iterations<<" iterations and fitness is "<<maxFitness;
                        cfg->logger.log(info, ss.str());
    			ss.str(std::string());
    			ss.clear();
                        result.cfg.copy(population[j]);
                        cfg->logger.log(debug, "Copied population");

                        result.GA_count = GACounter;
                        result.LDA_count = LDACounter;
                        result.LDA_time = LDATotTime;

                        // stops GA
                        return result;
                    }
                    maxFitnessChromosome = j;
                }
            }

            if(checkLowThreshold) {
                // if best is this low, restart from zero
                if(maxFitness < 0) {
                    for (int q = 0; q<spanSize; q++){
                        newPopulation[q].set_max(MAX_TOPICS, MAX_ITERATIONS);
                        newPopulation[q].random(); // needs at least 1 iteration
                    }
                    break;
                }
                checkLowThreshold = false;
            }

            //copy the chromosome with high fitness to the next generation
            newPopulation[i].copy(population[maxFitnessChromosome]);
            population[maxFitnessChromosome].fitness_value = INT_MIN;

        }

        //perform crossover - to fill the rest of the 2/3rd of the initial Population
        for(int i = 0 ; i < spanSize  ; i++ ) {
            newPopulation[spanSize+i].set_max(MAX_TOPICS, MAX_ITERATIONS);
            newPopulation[2*spanSize+i].set_max(MAX_TOPICS, MAX_ITERATIONS);

            newPopulation[spanSize+i].number_of_topics = population[i].number_of_topics;
            newPopulation[spanSize+i].random_iteration();

            newPopulation[2*spanSize+i].random_topic();
            newPopulation[2*spanSize+i].number_of_iterations = population[i].number_of_iterations;
        }

        //substitute the initial population with the new population and continue
        delete[] population;
        population = newPopulation;

        t = clock() - t;
        ss<<"GA took " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms";
	cfg->logger.log(info, ss.str());
       	ss.str(std::string());
    	ss.clear(); 
   }

    result.GA_count = GACounter;
    result.LDA_count = LDACounter;
	result.LDA_time = LDATotTime;

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
