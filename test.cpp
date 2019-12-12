#include <iostream>
#include <time.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "scanner.h"
#include "cluster.h"

using namespace std;

void test(unordered_map<string, Article> articlesMa) {
    vector<Article> articlesInCluster;
    articlesInCluster.push_back(articlesMa["filename"]);
    cout<<articlesInCluster[0].name<<endl;
    cout<<articlesInCluster[0].keyWords<<endl;
}
int main(int argc, char* argv[]) {

    // Scanner scn("topic.txt");
    //
    // // TODO: improve tokenization
    // while ( scn.nextLine() ) {
    //     cout<<"int: "<<scn.nextInt()<<endl;
    //     cout<<"double: "<<scn.nextDouble()<<endl;
    //
    //     for (int i = 0; i < 20; i++) {
	// 		cout<<scn.nextWord()<<"; ";
	// 	}
    //     cout<<endl;
    //     if(!scn.hasNextLine()) cout<<"That's it!"<<endl;
    // }
    // scn.close();
    // cout<<"Scanner closed"<<endl;
    // cout<<scn.nextWord()<<endl;
    // cout<<scn.nextLine()<<endl;
    // cout<<scn.nextWord()<<endl;

    cout<< "Starting clustering"<<endl;


    vector<Cluster> clusters = ClusterManager::createClusters();

    // for(int i = 0; i<clusters.size(); i++) {
    //     cout<<"This is cluster: "<<clusters[i].clusterNo<<"\t";
    //     cout<<endl<<">>>keywords<<< ";
    //     for ( auto keywords = clusters[i].keywords.begin(); keywords != clusters[i].keywords.end(); ++keywords ) {
    //         cout << *keywords <<";";
    //     }
    //     cout<<endl<<">>>articles<<< ";
    //     for(int j=0; j<clusters[i].articles.size(); j++) {
    //         cout<<clusters[i].articles[j]<<";";
    //     }
    //     cout<<endl<<">>>sourceFiles<<< ";
    //     for(int j=0; j<clusters[i].sourceFiles.size(); j++) {
    //         cout<<clusters[i].sourceFiles[j]<<";";
    //     }
    //     cout<<endl;
    // }
    // getchar();

    cout<<endl<<"Creation completed. Starting separation."<<endl<<endl;

    unordered_map<string, Article> articlesMap;
    unordered_map<string, SourceFile> sourceFileMap;

    ifstream myfile("input.txt");
    string line;
    while(getline (myfile, line, '\n')) {
        string filename = line.substr(0, line.find("\tX"));
        string keywords = line.substr(line.find("\tX\t")+3);
        if(filename.find("$AAA$") != string::npos){
            articlesMap[filename] = Article(filename, keywords); // needs real values for testing
        }
        else{
            sourceFileMap[filename] = SourceFile(filename, keywords); // needs real values for testing
        }
    }

    cout<<endl<<"Separation completed. Starting cleaning."<<endl<<endl;

    cout<<endl<<"Stage 1"<<endl<<endl;
    clusters = ClusterManager::cleanCluster(clusters, articlesMap, sourceFileMap);

    // for(int i = 0; i<clusters.size(); i++) {
    //     cout<<"This is cluster: "<<clusters[i].clusterNo<<"\t";
    //     cout<<endl<<">>>keywords<<< ";
    //     for ( auto keywords = clusters[i].keywords.begin(); keywords != clusters[i].keywords.end(); ++keywords ) {
    //         cout << *keywords <<";";
    //     }
    //     cout<<endl<<">>>articles<<< ";
    //     for(int j=0; j<clusters[i].articles.size(); j++) {
    //         cout<<clusters[i].articles[j]<<";";
    //     }
    //     cout<<endl<<">>>sourceFiles<<< ";
    //     for(int j=0; j<clusters[i].sourceFiles.size(); j++) {
    //         cout<<clusters[i].sourceFiles[j]<<";";
    //     }
    //     cout<<endl;
    // }
    // getchar();

    cout<<endl<<"Stage 2"<<endl<<endl;
    clusters = ClusterManager::cleanSourceFileCluster(clusters, sourceFileMap);

    cout<<endl<<"Cleaning completed. Starting results."<<endl<<endl;

    for(int i = 0; i<clusters.size(); i++) {
        cout<<"This is cluster: "<<clusters[i].clusterNo<<"\t";
        // cout<<endl<<">>>keywords<<< ";
        // for ( auto keywords = clusters[i].keywords.begin(); keywords != clusters[i].keywords.end(); ++keywords ) {
        //     cout << *keywords <<";";
        // }
        // cout<<endl<<">>>articles<<< ";
        // for(int j=0; j<clusters[i].articles.size(); j++) {
        //     cout<<clusters[i].articles[j]<<";";
        // }
        cout<<endl<<">>>sourceFiles<<< ";
        for(int j=0; j<clusters[i].sourceFiles.size(); j++) {
            cout<<clusters[i].sourceFiles[j]<<";";
        }
        cout<<endl;
    }
}
