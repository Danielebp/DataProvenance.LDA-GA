#include "parallelizables.h"


void parallelizableStart(int thread_index, PopulationConfig population_cfg, int population_index, int numberOfDocuments, bool original_version) {
		try {
			// create an instance of the topic modelling class
			// TO DO:
			// TopicModelling tm = new TopicModelling();

			if (!original_version && population_cfg.fitness_value > 0.0f) {
				// we have already got the fitness value. For example: this is one of the best
				// chromosomes in last round
				cout<<"New thread " << thread_index
				<< ": Used one of the best chromosomes in last round. Don't have to call LDA algorithm again. "
				<< population_cfg.to_string() << "  ************************************************"<<endl;

				population_cfg.LDA_execution_milliseconds = -1;
				return;
			}
			cout << (original_version ? "Original" : "New") << " thread " << thread_index << " start running "
					<< population_cfg.to_string()<<endl;

			// invoke the LDA function
			// TO DO:
			// tm.LDA(population_cfg, false, population_index, original_version);

			// clustermatrix - matrix explaining the distribution of documents into
			// different topics
			// the distibution is written to a text file by the name "distribution.txt"
			double* clusterMatrix = new double[(numberOfDocuments - 1) *(population_cfg.number_of_topics)];
			if (original_version) {
				cout<<(original_version ? "Original" : "New") << " thread " << thread_index << " is about to sleep"<<endl;
				// TO DO:
				//Thread.sleep(2000);
				cout<<(original_version ? "Original" : "New") << " thread " << thread_index << " is out of sleep"<<endl;
			}

			// Map to save the documents that belong to each cluster
			// An arraylist multimap allows to save <Key, Value[]> combination
			// each topic will have a cluster
			multimap<int, int> clusterMap;

			// reading the values from distribution.txt and populating the cluster matrix
			int rowNumber = 0, columnNumber = 0;
			ifstream myfile (("distribution" + to_string(population_index) + ".txt"));
			string line;
			try {
				bool hasNext = (bool)getline (myfile, line, '\n');

				int documentCount = 0;
				// read the values for every document
				while (hasNext && documentCount < (numberOfDocuments - 1)) {
					Scanner scanner(line);
					rowNumber = scanner.nextInt();
					hasNext = (bool)getline(myfile, line, '\n');

					for (int z = 0; z < population_cfg.number_of_topics; z++) {
						columnNumber = scanner.nextInt();
						if (z == 0) {
							clusterMap.insert(pair<int, int>(columnNumber, rowNumber));
						}
						clusterMatrix[(rowNumber*(numberOfDocuments - 1))+columnNumber] = scanner.nextDouble();
					}
					documentCount = documentCount + 1;
				}
			} catch (exception &e) {
				cout<<e.what()<<endl;
			}

			// auxiliar variables for mapping
			int countByKey;
			multimap<int,int>::iterator lastKey;
			int k;

			// getting the centroid of each cluster by calculating the average of their
			// cluster distribution
			double* clusterCentroids = new double[population_cfg.number_of_topics*population_cfg.number_of_topics];
			lastKey = clusterMap.begin();
			countByKey = 0;
			k = 0;
			for (multimap<int,int>::iterator it=clusterMap.begin(); it!=clusterMap.end(); ++it){
				if((*it).first == (*lastKey).first){
					countByKey++;

					for (int y = 0; y < population_cfg.number_of_topics; y++) {
						clusterCentroids[(k*population_cfg.number_of_topics)+y] += clusterMatrix[((*it).second*(numberOfDocuments - 1))+y];
					}
				}
				else{
					for (int y = 0; y < population_cfg.number_of_topics; y++) {
						clusterCentroids[(k*population_cfg.number_of_topics)+y] /= countByKey;
					}
					k++;
					lastKey = it;
					countByKey = 1;

					for (int y = 0; y < population_cfg.number_of_topics; y++) {
						clusterCentroids[(k*population_cfg.number_of_topics)+y] += clusterMatrix[((*it).second*(numberOfDocuments - 1))+y];
					}
				}
			}
			// process last key
			for (int y = 0; y < population_cfg.number_of_topics; y++) {
				clusterCentroids[(k*population_cfg.number_of_topics)+y] /= countByKey;
			}

			// finding the distance of each documents in each cluster finding max distance
			// from other documents in the same cluster
			double* maxDistanceInsideCluster = new double[numberOfDocuments - 1];
			lastKey = clusterMap.begin();
			maxDistanceInsideCluster[(*lastKey).first] = 0;
			for (multimap<int,int>::iterator it=next(clusterMap.begin()); it!=clusterMap.end(); ++it){
				int docNo = (*lastKey).second;
				int otherDocNo = (*it).second;
				double distance = 0;

				if((*it).first == (*lastKey).first){
					for (int h = 0; h < population_cfg.number_of_topics; h++) {
						distance += pow((clusterMatrix[(otherDocNo*(numberOfDocuments - 1))+h] - clusterMatrix[(docNo*(numberOfDocuments - 1))+h]), 2);
					}
					distance = sqrt(distance);
					if (distance > maxDistanceInsideCluster[docNo]) {
						maxDistanceInsideCluster[docNo] = distance;
					}
				}
				else {
					for (lastKey++; lastKey != it; lastKey++) {
						int docNo = (*lastKey).second;
						maxDistanceInsideCluster[docNo] = 0;
						for (multimap<int,int>::iterator z = next(lastKey); z != it; z++) {
							int otherDocNo = (*z).second;

							double distance = 0;
							for (int h = 0; h < population_cfg.number_of_topics; h++) {
								distance += pow((clusterMatrix[(otherDocNo*(numberOfDocuments - 1))+h] - clusterMatrix[(docNo*(numberOfDocuments - 1))+h]), 2);
							}
							distance = sqrt(distance);
							if (distance > maxDistanceInsideCluster[docNo]) {
								maxDistanceInsideCluster[docNo] = distance;
							}
						}
					}

					maxDistanceInsideCluster[(*it).first] = 0;
					lastKey = it;
				}
			}
			// process last key
			for (lastKey++; lastKey != clusterMap.end(); lastKey++) {
				int docNo = (*lastKey).second;
				maxDistanceInsideCluster[docNo] = 0;
				for (multimap<int,int>::iterator z = next(lastKey); z != clusterMap.end(); z++) {
					int otherDocNo = (*z).second;

					double distance = 0;
					for (int h = 0; h < population_cfg.number_of_topics; h++) {
						distance += pow((clusterMatrix[(otherDocNo*(numberOfDocuments - 1))+h] - clusterMatrix[(docNo*(numberOfDocuments - 1))+h]), 2);
					}
					distance = sqrt(distance);
					if (distance > maxDistanceInsideCluster[docNo]) {
						maxDistanceInsideCluster[docNo] = distance;
					}
				}
			}

			// continue from here

			// finding each documents minimum distance to the centroids of other clusters
			double* minDistanceOutsideCluster = new double[numberOfDocuments - 1];
			lastKey = clusterMap.begin();
			k = 0;
			minDistanceOutsideCluster[(*lastKey).second] = INT_MAX;

			for (multimap<int,int>::iterator it=clusterMap.begin(); it!=clusterMap.end(); ++it){
				int docNo = (*lastKey).second;

				if((*it).first == (*lastKey).first){

					for (int z = 0; z < population_cfg.number_of_topics; z++) {

						// don't calculate the distance to the same cluster
						if (z == k) {
							continue;
						}
						double distance = 0;
						for (int h = 0; h < population_cfg.number_of_topics; h++) {
							distance = distance + pow((clusterCentroids[(z*population_cfg.number_of_topics)+h] - clusterMatrix[(docNo*(numberOfDocuments - 1))+h]), 2);
						}
						distance = sqrt(distance);
						if (distance < minDistanceOutsideCluster[docNo]) {
							minDistanceOutsideCluster[docNo] = distance;
						}
					}
				}
				else{
					lastKey = it;
					k++;
					minDistanceOutsideCluster[docNo] = INT_MAX;

				}
			}

			// calculate the Silhouette coefficient for each document
			double* silhouetteCoefficient = new double[numberOfDocuments - 1];
			for (int m = 0; m < (numberOfDocuments - 1); m++) {
				silhouetteCoefficient[m] = (minDistanceOutsideCluster[m] - maxDistanceInsideCluster[m]) / max(minDistanceOutsideCluster[m], maxDistanceInsideCluster[m]);
			}

			// find the average of the Silhouette coefficient of all the documents - fitness
			// criteria
			double total = 0;
			for (int m = 0; m < (numberOfDocuments - 1); m++) {
				total = total + silhouetteCoefficient[m];
			}
			population_cfg.fitness_value = total / (numberOfDocuments - 1);
			cout<<(original_version ? "Original" : "New") << " thread " << thread_index << " finish  " << population_cfg.to_string()<<endl;
		} catch (exception &e) {
			// TODO Auto-generated catch block
			cout<<e.what()<<endl;
		}
	}
