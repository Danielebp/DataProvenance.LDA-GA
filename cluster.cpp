#include "cluster.h"

vector<Cluster> ClusterManager::createClusters() {

	vector<Cluster> clusters;

	// read the topic.txt to identify the number of clusters and the number
	// of the cluster and the keywords belonging to each cluster
	Scanner sc;
	try {

		sc.open("topic.txt");


		// for every topic create a cluster, read the topic no and the
		// top 20 keywords associated with the topic
		while (sc.nextLine()) {
			Cluster newCluster;
			newCluster.clusterNo = to_string(sc.nextInt());
			sc.nextDouble();
			for (int i = 0; i < 20; i++) {
				newCluster.keywords.insert(sc.nextWord());
			}
			clusters.push_back(newCluster);

		}
    } catch (exception& e) {

		cout<<"Hit error while reading the topic.txt "<<endl;
        cout<<e.what()<<endl;
	}
    sc.close();

	// System.out.println("Successfully scanned the file");
	// read the distribution.txt to find which file belongs to which topic
	try {

		sc.open("distribution.txt");


		// in every row there is a document and there is the proportional distribution
		// of the document
		// The first topic number is the topic the document belongs to
		while (sc.nextLine()) {
			// read the third string or int which is the topic number
			sc.nextInt();
			string name = sc.nextWord(); // the document name
			int topicNo = sc.nextInt(); // he topic it belongs to

			// see if the document is an article or source file by seeing the name
			if (name.find("$AAA$") != string::npos) {
				clusters[topicNo].articles.push_back(name);
			} else {
				clusters[topicNo].sourceFiles.push_back(name);
			}
		}
	} catch (exception& e) {

		cout<<"Hit error while reading the distribution.txt "<<endl;
        cout<<e.what()<<endl;
	}
    sc.close();

	// System.out.println("returning clusters");
	return clusters;

}


// there might be some clusters which have onyl source files and no articles
// the source files in such clusters should be distributed to clusters with
// articles
// the keywords that are associated with those clusters need to be found
// the source file can be transferred to the cluster with which it matches the
// most
vector<Cluster> ClusterManager::cleanSourceFileCluster(vector<Cluster> clusters, unordered_map<string, SourceFile> sourceFileMap) {

    int clusterNo = 0;

    // collect all the source files from clusters without an article
    vector<SourceFile> sourceFiles;

    // go through all the clusters
    for ( auto it = clusters.begin(); it != clusters.end();){
        //while (clusterNo < clusters.size()) {
        //Cluster cl = clusters[clusterNo];
        Cluster cl = *it;

        // check if the cluster has atleast one article
        // at this point the clusters cannot have more than one article

        if (cl.articles.size() > 1) {
            cout<<">>>>>Cluster has more than one article."<<endl;
            it++;
        } else if (cl.articles.size() == 1) {
            it++;
        } else {
            // if it has no articles then

            for (int i=0; i < cl.sourceFiles.size(); i++) {
                if(sourceFileMap.find(cl.sourceFiles[i]) == sourceFileMap.end()){
                    // this should never happen if clusters were generated from same sourceFiles
                    cout<<"SourceFile not found"<<endl;
                }
                else {
                    SourceFile sf = sourceFileMap[cl.sourceFiles[i]];
                    sourceFiles.push_back(sf);
                }
            }

            it = clusters.erase(it);
        }
    }

    // once all the source files have been collected figure out which cluster these
    // files belong to
    for (int i = 0; i < sourceFiles.size(); i++) {
        int maxMatch = INT_MIN;
        int maxMatchClusterNo = 0;

        // get all the keywords of the source file
        vector<string> keywords = split(sourceFiles[i].keyWords, ' ');

        // find how it matches with the keywords of each of the clusters
        for (int j = 0; j < clusters.size(); j++) {
            int count = 0;
            for (int k = 0; k < keywords.size(); k++) {
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
vector<Cluster> ClusterManager::cleanCluster(vector<Cluster> clusters, unordered_map<string, Article> articleMap, unordered_map<string, SourceFile> sourceFileMap) {
        int clusterNo = 0;
		// this is to make sure all the clusters are checked
		while (clusterNo < clusters.size()) {
        //for ( auto clusterIt = clusters.begin(); clusterIt != clusters.end(); ){

			// get each cluster
			Cluster cluster = clusters[clusterNo];
            cout<<"Checking cluster "<<cluster.clusterNo<<endl;

			// check if the cluster has 1 article or more than 1 article
			if (cluster.articles.size() <= 1) {

				// go check the next cluster
                clusterNo++;
			} else {

				// get the articles of this cluster
                vector<Article> articlesInCluster;

				for (int i = 0; i < cluster.articles.size(); i++) {
                    if(articleMap.find(cluster.articles[i]) == articleMap.end()){
                        // this should never happen if clusters were generated from same articlesMap
                        cout<<"Article not found"<<endl;
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
                // cout<<"Iterate over articlesMap"<<endl;
				for (int i = 0; i < articlesInCluster.size(); i++) {
					// cout<<articlesInCluster[i].name<<endl;
					// cout<<articlesInCluster[i].getKeyWords()<<endl;
                    vector<string> keywordArray = split(articlesInCluster[i].keyWords, ' ');
					unordered_set<string> keyWordSet (keywordArray.begin(), keywordArray.end());
					articlesInCluster[i].uniqueKeyWordSet = keyWordSet;

					// for every keywords add it to the hashtable
					for (int j = 0; j < keywordArray.size(); j++) {
						articlesInCluster[i].totalWordCount = articlesInCluster[i].totalWordCount + 1;
						if (articlesInCluster[i].uniqueKeyWords.find(keywordArray[j]) != articlesInCluster[i].uniqueKeyWords.end()) {
							articlesInCluster[i].uniqueKeyWords[keywordArray[j]] = articlesInCluster[i].uniqueKeyWords[keywordArray[j]] + 1;
						} else {
							articlesInCluster[i].uniqueKeyWords[keywordArray[j]] = 1;
						}
					}
				}

                // cout<<"Remove common words"<<endl;
				// then remove the words from the articles that belong to other articles to
				// now remove all the common keywords that are there between any two articles
				// the articles should be left with keywords that are soleley special to them
				// and do not overlap with the keywords of any other article
				for (int i = 0; i < articlesInCluster.size(); i++) {
					for (int j = i + 1; j < articlesInCluster.size(); j++) {
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
                // cout<<"Remove words with lower occurence"<<endl;
				for (int i = 0; i < articlesInCluster.size(); i++) {

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

                // cout<<"Backup source files"<<endl;
				// get the list of source files in the cluster
				vector<SourceFile> sourceFilesInCluster;
				// retrieve the sourcefiles from the SourceFileMap
				for (int i = 0; i < cluster.sourceFiles.size(); i++) {
                    if(sourceFileMap.find(cluster.sourceFiles[i]) == sourceFileMap.end()){
                        // this should never happen if clusters were generated from same sourceFiles
                        cout<<"SourceFile not found"<<endl;
                        cout<<cluster.sourceFiles[i]<<endl;
                    }
                    else {
    					SourceFile source = sourceFileMap[cluster.sourceFiles[i]];
    					sourceFilesInCluster.push_back(source);
                    }
				}

				// create a new cluster for each of the article
				// add the name of the article to the article list
				// add this to the new cluster list

                // cout<<"Create new clusters"<<endl;
				vector<Cluster> newClusterList;
				for (int i = 0; i < articlesInCluster.size(); i++) {
					Cluster newCluster;
					newCluster.clusterNo = cluster.clusterNo + "_" + to_string(i);
					newCluster.keywords = (unordered_set<string>) articlesInCluster[i].uniqueKeyWordSet;
					newCluster.articles.push_back(articlesInCluster[i].name);
					newClusterList.push_back(newCluster);
				}

				// remove the old cluster from the clusters list
				clusters.erase(clusters.begin() + clusterNo);

                // cout<<"Re-distribute source files"<<endl;
				// for each source file find the amount of overlap it has with each of the
				// article
				for (int i = 0; i < sourceFilesInCluster.size(); i++) {
					// cout<<sourceFilesInCluster[i].getName()<<endl;
					int max = -1;
					int clusterOverLapNo = -1;
					// compare the amount of overlap of the source file with each of the articles
					for (int j = 0; j < articlesInCluster.size(); j++) {
						int count = 0;
                        for ( auto it = articlesInCluster[j].uniqueKeyWordSet.begin(); it != articlesInCluster[j].uniqueKeyWordSet.end(); ++it ){
							if (sourceFilesInCluster[i].keyWords.find(" " + (*it) + " ") != string::npos) {
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



                // cout<<"Update clusters list"<<endl;
                for (auto it = newClusterList.begin(); it < newClusterList.end(); it++) {
                    // cout<<clusters.size()<<" - "<<newClusterList.size()<<endl;
					clusters.push_back(*it);
				}

			}

		}
        // cout<<"Finish stage 1 cleanup"<<endl;
        return clusters;
	}
