#ifndef __cluster_h__
#define __cluster_h__

#include <string>
#include <iostream>
#include <time.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <math.h>
#include <limits.h>
#include "utils.h"
#include "scanner.h"
#include "sourceFile.h"
#include "article.h"
#include "config.h"

using namespace std;

struct Cluster
{
    // each cluster is supposed to have one article and a number of source files
	// associated with it
	/// there might be some clusters which have 2 articles
	// these clusters will have to be split into smaller clusters
    vector<string> articles;
    vector<string> sourceFiles;

	// the keywords that are associated with this cluster
	// these keywords are to be read from the topic.txt
    unordered_set<string> keywords;

	// each topic will and cluster will also be given a cluster/topicno
	string clusterNo;
};

class ClusterManager{

public:
    // reads the data from the topic.txt and distribution.txt
	// create cluster for each topic
	// assigns the keywords to the appropraite topic
    static vector<Cluster> createClusters(ConfigOptions* cfg) ;
    static vector<Cluster> cleanSourceFileCluster(vector<Cluster>& clusters, unordered_map<string, SourceFile>& sourceFileMap, ConfigOptions* cfg);
    static vector<Cluster> cleanCluster(vector<Cluster>& clusters, unordered_map<string, Article>& articleMap, unordered_map<string, SourceFile>& sourceFileMap, ConfigOptions* cfg);
	// TODO: cleanClusters ()

};

#endif
