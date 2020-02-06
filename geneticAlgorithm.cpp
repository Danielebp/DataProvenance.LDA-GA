
#include "geneticAlgorithm.h"


ResultStatistics calculatePrecisionRecall(ResultStatistics result, vector<Cluster> clusters, bool debug) {

	if (clusters.size() <= 0) {
        result.precision_percentage = 0;
    	result.recall_percentage = 0;
		return result;
	}
    string line;
    float* precision = new float[clusters.size()];
    float* recall = new float[clusters.size()];
    unordered_map<string,string> truthData;
    ifstream myfile ("truthfile.txt");
    int stringCheck;

    while ( getline (myfile,line) ) {
        stringCheck = line.find("#");
        string split = line.substr(0, stringCheck);
        truthData[line.substr(0, stringCheck)] = line.substr(stringCheck+2);
    }
    if(debug) cout<<"Finished reading truthfile"<<endl;

	for (int i = 0; i < clusters.size(); i++) {
        if(debug) cout<<"Cluster "<<i<<endl;
		precision[i] = 0;
		recall[i] = 0;

		Cluster cl = clusters[i];

        // at this point there should be only one article per cluster
		string name = cl.articles[0];
		vector<string> sources = cl.sourceFiles;

		// retrieve the article from the truth file
		string trueSource = truthData[name];
		if (trueSource == "") {
			cout<<"Failed to find truth data: " + name<<endl;
			continue;
		}
		vector<string> trueSourceSplit = split(trueSource, ' ');
        if(debug) cout<<"Done with split"<<endl;

		// calculating precision
		if (sources.size() != 0) {
			int precise_count = 0;
			for (int j = 0; j < sources.size(); j++) {
				if (trueSource.find(sources[j])!=string::npos) {
					precise_count++;
				}
			}
			precision[i] = (float) precise_count / (float) sources.size();
		}
        if(debug) cout<<"Done with precision"<<endl;

		// calculate recall
		// convert the list of source files to a set
		unordered_set<string> sourceSet;
		for (int j = 0; j<sources.size(); j++) {
			sourceSet.insert(sources[j]);
		}
        if(debug) cout<<"Done converting"<<endl;

		if (trueSourceSplit.size() <= 0) {
			if (sourceSet.find(trueSource) != sourceSet.end()) {
				recall[i] = 1;
			}
		} else {
			int recall_count = 0;
			for (int j = 0; j < trueSourceSplit.size(); j++) {
				if (sourceSet.find(trueSourceSplit[j]) != sourceSet.end()) {
					recall_count++;
				}
			}
			recall[i] = (float) recall_count / (float) trueSourceSplit.size();
		}
        if(debug) cout<<"Done with recall"<<endl;
	}
    if(debug) cout<<"Finished calculating partials"<<endl;

	// calculating the average precision and recall
	float precision_total = 0.0;
	float recall_total = 0.0;

	for (int i = 0; i < clusters.size(); i++) {
		precision_total += precision[i];
		recall_total += recall[i];
	}
    if(debug) cout<<"Finished calculating totals"<<endl;

	result.precision_percentage = (precision_total / clusters.size()) * 100;
	result.recall_percentage = (recall_total / clusters.size()) * 100;

    return result;
}


ResultStatistics geneticLogic(int populationSize, int numberOfDocuments, double fitnessThreshold, bool debug, bool progress) {
    ResultStatistics result;
    PopulationConfig* population = new PopulationConfig[populationSize];

    unordered_map<string, Article> articlesMap;
    unordered_map<string, SourceFile> sourceFileMap;

    ifstream myfile("input1.txt");
    string line;
    while(getline (myfile, line, '\n')) {
        string filename = line.substr(0, line.find("##lda_delimiter##"));
        string keywords = line.substr(line.find("##lda_delimiter##")+17);
        if(filename.find("$AAA$") != string::npos){
            articlesMap[filename] = Article(filename, keywords); // needs real values for testing
        }
        else{
            sourceFileMap[filename] = SourceFile(filename, keywords); // needs real values for testing
        }
    }

    if(debug) cout<<"Starting GA"<<endl;
    if(debug) cout<<"###########################"<<endl;
    clock_t preprocessEndTime = clock();

    // initialize population
    for (int i = 0; i<populationSize; i++){
        population[i].set_max(MAX_TOPICS, MAX_ITERATIONS); // define max of topics and iterations
        population[i].random(); // generate random number of topics and iterations
    }

    int GACounter = 0;
    int LDACounter = 0;

    clock_t t;
    clock_t total_t = clock();

    bool maxFitnessFound = false;
    long LDATotTime = 0;

    clock_t exTm = clock();
    while (!maxFitnessFound){
        GACounter ++;
        // runs population
        if(progress) cout<<"GA Attempt: "<<GACounter<< " - Fitness Threshold: " << fitnessThreshold <<endl;

        bool checkLowThreshold = true;
        for (int i = 0; i<populationSize; i++){
            LDACounter ++;

            t = clock();

            TopicModelling tm(population[i].number_of_topics, population[i].number_of_iterations, numberOfDocuments, debug);

            string fileToDelete = "__"+to_string(i)+"__"+to_string(population[i].number_of_topics)+"x"+to_string(population[i].number_of_topics);
            tm.LDA(fileToDelete);
            t = clock() - t;

            population[i].LDA_execution_milliseconds = ((float)t)/(CLOCKS_PER_SEC/1000);
            LDATotTime += population[i].LDA_execution_milliseconds;
            int numberOfTopics = population[i].number_of_topics;

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
                    if(debug) cout<<tm.getDocNameByNumber(d)<<endl;
                }
            }
            if(debug) cout<<"Documents without topic: "<<topicLess<<endl;

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
            population[i].fitness_value = total / (numberOfDocuments - topicLess);
            if(progress) cout<<"LDA Attempt: "<<LDACounter<<" - Fitness: "<<population[i].fitness_value<<endl;

            if(population[i].fitness_value >= fitnessThreshold) {
                tm.WriteFiles();
                break;
            }

        }


        t = clock();

        //ranking and ordering the chromosomes based on the fitness function.
        //no sorting code found?(by Xiaolin)
        //We need only the top 1/3rd of the chromosomes with high fitness values - Silhouette coefficient
        PopulationConfig* newPopulation = new PopulationConfig[populationSize];
        int spanSize = populationSize/3;
        //copy only the top 1/3rd of the chromosomes to the new population
        for(int i = 0 ; i < spanSize ; i++) {
            double maxFitness = INT_MIN;
            int maxFitnessChromosome = -1;
            for(int j = 0 ; j < populationSize ; j++) {
                if(population[j].fitness_value > maxFitness) {
                    maxFitness = population[j].fitness_value;

                    // when maxFitness satisfies the requirement, stop running GA
                    if(maxFitness >= fitnessThreshold) {

                        // TODO: running the LDA again, which is not such a great idea, see if it can be removed
                        TopicModelling tm(population[j].number_of_topics, population[j].number_of_iterations, numberOfDocuments, debug);
                        tm.LDA("");
                        tm.WriteFiles();

                        //run the function again to get the words in each topic
                        cout<<"the best distribution is "<<population[j].number_of_topics<<" topics and "<<population[j].number_of_iterations<<" iterations and fitness is "<<maxFitness<<endl;
                        maxFitnessFound = true;
                        result.cfg.copy(population[j]);
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

        if(maxFitnessFound) {
            break;
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
        cout << "GA took " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms"<<endl;
    }

    // ######################## HERE #############################
    // TODO: finish this part
    // Outputs the time it took to finish the genetic algorithm
    clock_t geneticEndTime = clock();
    cout<<"Genetic algorithm takes " << (geneticEndTime - preprocessEndTime) << "ms"<<endl;

    // create clusters based on the distribution.txt
	vector<Cluster> clusters = ClusterManager::createClusters();
    if(debug) cout<<"Cluster created"<<endl;

	// by cleaning the clusters
	// we got through the obtained list of clusters
	// check for conditions where there are more than 2 articles in the same cluster
	// perform the job of splitting the cluster into 2
	clusters = ClusterManager::cleanCluster(clusters, articlesMap, sourceFileMap);
    if(debug) cout<<"Cluster Cleaned"<<endl;


	// there might be some clusters with no article in them but all source files
	// to handle that we use the following technique/function
	clusters = ClusterManager::cleanSourceFileCluster(clusters, sourceFileMap);
    if(debug) cout<<"Cluster Sources cleaned"<<endl;


	clock_t clusteringEndTime = clock();
	cout<<"Clustering takes " << (clusteringEndTime - geneticEndTime) << "ms"<<endl;

    result = calculatePrecisionRecall(result, clusters, debug);
    if(debug) cout<<"Calculated precision & recall"<<endl;

    exTm = clock() - exTm;

    result.GA_count = GACounter;
    result.LDA_count = LDACounter;
	result.LDA_time = LDATotTime;
    result.execution_milliseconds = ((float)exTm)/(CLOCKS_PER_SEC/1000);
    cout<<result.to_string("")<<endl;

    total_t = clock() - total_t;

    cout<<endl<<"###########################################"<<endl;
    cout<<"Number of GA iterations: " << GACounter<<endl;

    // ###########################################################

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
