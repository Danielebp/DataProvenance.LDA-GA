#include "cluster.h"

vector<Cluster> ClusterManager::createClusters(ConfigOptions* cfg) {
    vector<Cluster> clusters;

    cfg->logger.log(debug, "Start reading files for cluster creation");

	// read the topic.txt to identify the number of clusters and the number
	// of the cluster and the keywords belonging to each cluster
	Scanner sc;
	try {

		sc.open(cfg->outputDir + "/topic.txt");


		// for every topic create a cluster, read the topic no and the
		// top 20 keywords associated with the topic
		while (sc.hasNextLine()) {
            sc.nextLine();
			Cluster newCluster;
            int clusterNo = sc.nextInt();
			newCluster.clusterNo = to_string(clusterNo);
			sc.nextDouble();
			for (int i = 0; i < 20; i++) {
				newCluster.keywords.insert(sc.nextWord());
			}
			clusters.push_back(newCluster);
		}
    } catch (exception& e) {

		cfg->logger.log(error, "Hit error while reading the topic.txt ");
        cfg->logger.log(error, e.what());
	}
    sc.close();

    cfg->logger.log(debug, "Finished reading topic.txt");

	// System.out.println("Successfully scanned the file");
	// read the distribution.txt to find which file belongs to which topic
	try {

		sc.open(cfg->outputDir + "/distribution.txt");

		// in every row there is a document and there is the proportional distribution
		// of the document
		// The first topic number is the topic the document belongs to
		while (sc.nextLine()) {
			// read the third string or int which is the topic number
			// sc.nextInt();
			string name = sc.nextWord(); // the document name
			int topicNo = sc.nextInt(); // the topic it belongs to

			// see if the document is an article or source file by seeing the name
			if (name.find("$AAA$") != string::npos) {
				clusters[topicNo].articles.push_back(name);
			} else {
				clusters[topicNo].sourceFiles.push_back(name);
			}
		}
	} catch (exception& e) {

		cfg->logger.log(error, "Hit error while reading the distribution.txt ");
        cfg->logger.log(error, e.what());
	}
    sc.close();
    cfg->logger.log(debug, "Finished reading distribution.txt");

	// System.out.println("returning clusters");
	return clusters;

}


// there might be some clusters which have onyl source files and no articles
// the source files in such clusters should be distributed to clusters with
// articles
// the keywords that are associated with those clusters need to be found
// the source file can be transferred to the cluster with which it matches the
// most
vector<Cluster> ClusterManager::cleanSourceFileCluster(vector<Cluster>& _clusters, unordered_map<string, SourceFile>& sourceFileMap, ConfigOptions* cfg) {

    stringstream ss;
    vector<Cluster> clusters;
    clusters.reserve(_clusters.size());

    // collect all the source files from clusters without an article
    vector<SourceFile> sourceFiles;

    cfg->logger.log(debug, "Going over clusters");
    // go through all the clusters
    for ( auto it = _clusters.begin(); it != _clusters.end();){
        //while (clusterNo < clusters.size()) {
        //Cluster cl = clusters[clusterNo];
        Cluster cl = *it;

        // check if the cluster has atleast one article
        // at this point the clusters cannot have more than one article

        if (cl.articles.size() > 1) {
            cfg->logger.log(info, ">>>>>Cluster has more than one article.");
            clusters.push_back(cl);
        } else if (cl.articles.size() == 1) {
            clusters.push_back(cl);
        } else {
            // if it has no articles then

            // allocates enough memory for all sources
            sourceFiles.reserve(sourceFiles.size() + cl.sourceFiles.size());
            for (unsigned int i=0; i < cl.sourceFiles.size(); i++) {
                if(sourceFileMap.find(cl.sourceFiles[i]) == sourceFileMap.end()){
                    // this should never happen if clusters were generated from same sourceFiles
                    ss<<"SourceFile >>"<<cl.sourceFiles[i]<<"<< not found";
                    cfg->logger.log(info, ss.str());
                    ss.str(std::string());
                    ss.clear();
                }
                else {
                    SourceFile sf = sourceFileMap[cl.sourceFiles[i]];
                    sourceFiles.push_back(sf);
                }
            }
            // does not add cluster
            //it = clusters.erase(it);
        }
            it++;
    }
    clusters.shrink_to_fit();
    cfg->logger.log(debug, "Went over clusters, now re-distributing sources");
    // once all the source files have been collected figure out which cluster these
    // files belong to
    for (unsigned int i = 0; i < sourceFiles.size(); i++) {
        int maxMatch = INT_MIN;
        int maxMatchClusterNo = 0;

        cfg->logger.log(debug, "Source " + to_string(i) + "/" + to_string(sourceFiles.size()));
        // get all the keywords of the source file
        vector<string> keywords = split(sourceFiles[i].keyWords, ' ');

        // find how it matches with the keywords of each of the clusters
        for (unsigned int j = 0; j < clusters.size(); j++) {
            int count = 0;
            for (unsigned int k = 0; k < keywords.size(); k++) {
                if (clusters[j].keywords.find(keywords[k]) != clusters[j].keywords.end()) {
                    count++;
                }
            }

            // find the cluster with which it matches the most
            if (count > maxMatch) {
                maxMatch = count;
                maxMatchClusterNo = j;
            }
        }

        // assign the source file to that cluster
        clusters[maxMatchClusterNo].sourceFiles.push_back(sourceFiles[i].name);

    }

    return clusters;

}


// TODO: Cluster::cleanCluster()
// find clusters that have 2 articles in them
	// pick these clusters and identify the words that are unique to each of these
	// articles
	// classify the source files into these articles
	// create new cluster for each of these article and add them to the main list,
	// remove the cluster which had more then one article from the main list
	// "clusters"
vector<Cluster> ClusterManager::cleanCluster(vector<Cluster>& clusters, unordered_map<string, Article>& articleMap, unordered_map<string, SourceFile>& sourceFileMap, ConfigOptions* cfg) {
        vector<Cluster> newClusterList;
        newClusterList.reserve(articleMap.size());
        stringstream ss;

		// this is to make sure all the clusters are checked
        for ( auto it = clusters.begin(); it != clusters.end(); it++){
            //while (clusterNo < clusters.size()) {
            //Cluster cl = clusters[clusterNo];
            Cluster cluster = *it;
            //while (clusterNo < clusters.size()) {
            //for ( auto clusterIt = clusters.begin(); clusterIt != clusters.end(); ){

			// get each cluster
			//Cluster cluster = clusters[clusterNo];
	    ss<<"Checking cluster "<<cluster.clusterNo;
            cfg->logger.log(debug, ss.str());
            ss.str(std::string());
            ss.clear();

			// check if the cluster has 1 article or more than 1 article
			if (cluster.articles.size() <= 1) {
                cfg->logger.log(debug, "1 article");
				// go check the next cluster
                newClusterList.push_back(cluster);
			} else {
                cfg->logger.log(debug, "multi articles");

				// get the articles of this cluster
                vector<Article> articlesInCluster;
                articlesInCluster.reserve(cluster.articles.size());

				for (unsigned int i = 0; i < cluster.articles.size(); i++) {
                    if(articleMap.find(cluster.articles[i]) == articleMap.end()){
                        // this should never happen if clusters were generated from same articlesMap
                        ss<<""<<cluster.articles[i];
                        cfg->logger.log(info, ss.str());
                        ss.str(std::string());
                        ss.clear();
                    }
                    else {
	                    articlesInCluster.push_back(articleMap[cluster.articles[i]]);
                    }
				}

				// for each of the articles in the cluster
				// count the number of occurrence of each word
				// find the unique keywords as well
				// in the same time also count the total number of words in this article
				// so instead of having the unique words as a set, lets change the unique
				// words into a hashmap
        cfg->logger.log(debug, "Iterate over articlesMap");
				for (unsigned int i = 0; i < articlesInCluster.size(); i++) {
					// cout<<articlesInCluster[i].name<<endl;
					// cout<<articlesInCluster[i].getKeyWords()<<endl;
                    vector<string> keywordArray = split(articlesInCluster[i].keyWords, ' ');
					unordered_set<string> keyWordSet (keywordArray.begin(), keywordArray.end());
					articlesInCluster[i].uniqueKeyWordSet = keyWordSet;

					// for every keywords add it to the hashtable
					for (unsigned int j = 0; j < keywordArray.size(); j++) {
						articlesInCluster[i].totalWordCount = articlesInCluster[i].totalWordCount + 1;
						if (articlesInCluster[i].uniqueKeyWords.find(keywordArray[j]) != articlesInCluster[i].uniqueKeyWords.end()) {
							articlesInCluster[i].uniqueKeyWords[keywordArray[j]] = articlesInCluster[i].uniqueKeyWords[keywordArray[j]] + 1;
						} else {
							articlesInCluster[i].uniqueKeyWords[keywordArray[j]] = 1;
						}
					}
				}

        cfg->logger.log(debug, "Remove common words");
				// then remove the words from the articles that belong to other articles to
				// now remove all the common keywords that are there between any two articles
				// the articles should be left with keywords that are soleley special to them
				// and do not overlap with the keywords of any other article
				for (unsigned int i = 0; i < articlesInCluster.size(); i++) {
					for (unsigned int j = i + 1; j < articlesInCluster.size(); j++) {
                        for ( auto it = articlesInCluster[j].uniqueKeyWordSet.begin(); it != articlesInCluster[j].uniqueKeyWordSet.end(); ++it ){
                            if(articlesInCluster[i].uniqueKeyWordSet.find(*it)!= articlesInCluster[i].uniqueKeyWordSet.end())articlesInCluster[i].uniqueKeyWordSet.erase(*it);
                        }
                        for ( auto it = articlesInCluster[i].uniqueKeyWordSet.begin(); it != articlesInCluster[i].uniqueKeyWordSet.end(); ++it ){
                            if(articlesInCluster[j].uniqueKeyWordSet.find(*it)!= articlesInCluster[j].uniqueKeyWordSet.end())articlesInCluster[j].uniqueKeyWordSet.erase(*it);
                        }
					}
				}

				// go through the unique keyword and identify the percentage of occurence of
				// each word
				// let us assume the percentage of occurrence needs to be more than 1.5%
				// remove the words that occur less than the threshold

				// for each of the articles
        cfg->logger.log(debug, "Remove words with lower occurence");
				for (unsigned int i = 0; i < articlesInCluster.size(); i++) {

					// for every unique word in the article
                    for ( auto it = articlesInCluster[i].uniqueKeyWordSet.begin(); it != articlesInCluster[i].uniqueKeyWordSet.end(); ++it ){
						// get the count of it from the hashtable
						int count = articlesInCluster[i].uniqueKeyWords[*it];
						float percentage = (float) count / (float) articlesInCluster[i].totalWordCount;

						// if the percentage does not meet the threshold, remove the word from the set
						if (percentage < 1.5) {
                            articlesInCluster[i].uniqueKeyWords.erase(*it);
						}
					}
				}

       cfg->logger.log(debug, "Backup source files");
				// get the list of source files in the cluster
				vector<SourceFile> sourceFilesInCluster;
        sourceFilesInCluster.reserve(cluster.sourceFiles.size());
				// retrieve the sourcefiles from the SourceFileMap
				for (unsigned int i = 0; i < cluster.sourceFiles.size(); i++) {
                    if(sourceFileMap.find(cluster.sourceFiles[i]) == sourceFileMap.end()){
                        // this should never happen if clusters were generated from same sourceFiles
                        ss<<"SourceFile not found: "<<cluster.sourceFiles[i];
                        cfg->logger.log(error, ss.str());
                        ss.str(std::string());
                        ss.clear();
                    }
                    else {
    					SourceFile source = sourceFileMap[cluster.sourceFiles[i]];
    					sourceFilesInCluster.push_back(source);
                    }
				}

				// create a new cluster for each of the article
				// add the name of the article to the article list
				// add this to the new cluster list

        cfg->logger.log(debug, "Create new clusters");
				for (unsigned int i = 0; i < articlesInCluster.size(); i++) {
					Cluster newCluster;
					newCluster.clusterNo = cluster.clusterNo + "_" + to_string(i);
					newCluster.keywords = (unordered_set<string>) articlesInCluster[i].uniqueKeyWordSet;
					newCluster.articles.push_back(articlesInCluster[i].name);
					newClusterList.push_back(newCluster);
				}


        cfg->logger.log(debug, "Re-distribute source files");
				// for each source file find the amount of overlap it has with each of the
				// article
				for (unsigned int i = 0; i < sourceFilesInCluster.size(); i++) {
					// cout<<sourceFilesInCluster[i].getName()<<endl;
					int max = -1;
					int clusterOverLapNo = -1;
          vector<string> srcKeywordArray = split(sourceFilesInCluster[i].keyWords, ' ');
          unordered_set<string> srcKeyWordSet (srcKeywordArray.begin(), srcKeywordArray.end());
					// compare the amount of overlap of the source file with each of the articles
					for (unsigned int j = 0; j < articlesInCluster.size(); j++) {
						int count = 0;
                        for ( auto it = articlesInCluster[j].uniqueKeyWordSet.begin(); it != articlesInCluster[j].uniqueKeyWordSet.end(); ++it ){
							//if (sourceFilesInCluster[i].keyWords.find(" " + (*it) + " ") != string::npos) {
              if (srcKeyWordSet.find(*it) != srcKeyWordSet.end()){
								count = count + 1;
							}
						}

						if (count > max) {
							max = count;
							clusterOverLapNo = j;
						}
					}

					newClusterList[clusterOverLapNo].sourceFiles.push_back(sourceFilesInCluster[i].name);
				}


			}

		}
        cfg->logger.log(debug,"Finish stage 1 cleanup");
        return newClusterList;
	}
