
#include "dataProvenance.h"


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

//getting the centroid of each cluster by calculating the average of their cluster distribution

vector<Cluster> performClustering(unordered_map<string, Article> articlesMap,
                        unordered_map<string, SourceFile> sourceFileMap, bool debug, bool progress){
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

    return clusters;
}



ResultStatistics reconstructProvenance(int populationSize, int numberOfDocuments, double fitnessThreshold, bool cuda, bool debug, bool progress) {
    ResultStatistics result;

    unordered_map<string, Article> articlesMap;
    unordered_map<string, SourceFile> sourceFileMap;

    ifstream myfile("./tempData/input1.txt");
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
    clock_t exTm = clock();

    result = geneticLogic(populationSize, numberOfDocuments, fitnessThreshold, cuda, debug, progress);
    clock_t geneticEndTime = clock();
    cout<<"Genetic algorithm takes " << (geneticEndTime - exTm) << "ms"<<endl;

    // create clusters based on the distribution.txt
	vector<Cluster> clusters = performClustering(articlesMap, sourceFileMap, debug, progress);

	clock_t clusteringEndTime = clock();
	cout<<"Clustering takes " << (clusteringEndTime - geneticEndTime) << "ms"<<endl;

    result = calculatePrecisionRecall(result, clusters, debug);
    if(debug) cout<<"Calculated precision & recall"<<endl;

    exTm = clock() - exTm;
    result.execution_milliseconds = ((float)exTm)/(CLOCKS_PER_SEC/1000);
    cout<<result.to_string("")<<endl;
    cout<<endl<<"###########################################"<<endl;

    return result;
}
