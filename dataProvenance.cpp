
#include "dataProvenance.h"


ResultStatistics calculatePrecisionRecall(ResultStatistics result, vector<Cluster> clusters, ConfigOptions* cfg) {

	if (clusters.size() <= 0) {
        result.precision_percentage = 0;
    	result.recall_percentage = 0;
	cfg->logger.log(error, "No clusters were created");
		return result;
	}
    string line;
    float* precision = new float[clusters.size()];
    float* recall = new float[clusters.size()];
    unordered_map<string,string> truthData;
    ifstream myfile (cfg->truthFile);
    int stringCheck;

    while ( getline (myfile,line) ) {
        stringCheck = line.find("#");
        string split = line.substr(0, stringCheck);
        truthData[line.substr(0, stringCheck)] = line.substr(stringCheck+2);
    }
    cfg->logger.log(debug, "Finished reading truthfile");

	for (int i = 0; i < clusters.size(); i++) {
        cfg->logger.log(debug, "Cluster ");
		precision[i] = 0;
		recall[i] = 0;

		Cluster cl = clusters[i];

        // at this point there should be only one article per cluster
		string name = cl.articles[0];
		vector<string> sources = cl.sourceFiles;

		// retrieve the article from the truth file
		string trueSource = truthData[name];
		if (trueSource == "") {
			cfg->logger.log(error, "Failed to find truth data: " + name);
			continue;
		}
		vector<string> trueSourceSplit = split(trueSource, ' ');
        cfg->logger.log(debug, "Done with split");

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
        cfg->logger.log(debug, "Done with precision");

		// calculate recall
		// convert the list of source files to a set
		unordered_set<string> sourceSet;
		for (int j = 0; j<sources.size(); j++) {
			sourceSet.insert(sources[j]);
		}
        cfg->logger.log(debug, "Done converting");

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
        cfg->logger.log(debug, "Done with recall");
	}
    cfg->logger.log(debug, "Finished calculating partials");

	// calculating the average precision and recall
	float precision_total = 0.0;
	float recall_total = 0.0;

	for (int i = 0; i < clusters.size(); i++) {
		precision_total += precision[i];
		recall_total += recall[i];
	}
    cfg->logger.log(debug, "Finished calculating totals");

	result.precision_percentage = (precision_total / clusters.size()) * 100;
	result.recall_percentage = (recall_total / clusters.size()) * 100;

    return result;
}

//getting the centroid of each cluster by calculating the average of their cluster distribution

vector<Cluster> performClustering(unordered_map<string, Article> articlesMap,
                        unordered_map<string, SourceFile> sourceFileMap, ConfigOptions* cfg){
    // create clusters based on the distribution.txt
	vector<Cluster> clusters = ClusterManager::createClusters(cfg);
    cfg->logger.log(debug, "Cluster created");

	// by cleaning the clusters
	// we got through the obtained list of clusters
	// check for conditions where there are more than 2 articles in the same cluster
	// perform the job of splitting the cluster into 2
	clusters = ClusterManager::cleanCluster(clusters, articlesMap, sourceFileMap, cfg);
    cfg->logger.log(debug, "Cluster Cleaned");


	// there might be some clusters with no article in them but all source files
	// to handle that we use the following technique/function
	clusters = ClusterManager::cleanSourceFileCluster(clusters, sourceFileMap, cfg);
    cfg->logger.log(debug, "Cluster Sources cleaned");

    return clusters;
}



ResultStatistics reconstructProvenance(int numberOfDocuments, ConfigOptions * cfg) {
    ResultStatistics result;
    stringstream ss;
    unordered_map<string, Article> articlesMap;
    unordered_map<string, SourceFile> sourceFileMap;

    ifstream myfile(cfg->ldaInputFile);
    string line;
    while(getline (myfile, line, '\n')) {
        string filename = line.substr(0, line.find(cfg->delimiter));
        string keywords = line.substr(line.find(cfg->delimiter)+17);
        if(filename.find("$AAA$") != string::npos){
            articlesMap[filename] = Article(filename, keywords); // needs real values for testing
        }
        else{
            sourceFileMap[filename] = SourceFile(filename, keywords); // needs real values for testing
        }
    }
    clock_t exTm = clock();

    result = geneticLogic(numberOfDocuments, cfg);
    clock_t geneticEndTime = clock();

    ss<< "Genetic algorithm takes " << (geneticEndTime - exTm) << "ms";
    cfg->logger.log(status,ss.str());
    ss.str(std::string());
    ss.clear();

    // create clusters based on the distribution.txt
    vector<Cluster> clusters = performClustering(articlesMap, sourceFileMap, cfg);

    clock_t clusteringEndTime = clock();
    ss<<"Clustering takes " << (clusteringEndTime - geneticEndTime) << "ms";
    cfg->logger.log(status, ss.str());
    ss.str(std::string());
    ss.clear();

    result = calculatePrecisionRecall(result, clusters, cfg);
    cfg->logger.log(debug, "Calculated precision & recall");

    exTm = clock() - exTm;
    result.execution_milliseconds = ((float)exTm)/(CLOCKS_PER_SEC/1000);
    cfg->logger.log(status, result.to_string(""));
    cfg->logger.log(info, "###########################################");

    return result;
}
