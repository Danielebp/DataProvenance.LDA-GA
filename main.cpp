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
#include "document.h"
#include "utils.h"
#include "geneticAlgorithm.h"
#include "wordFilter.h"

using namespace std;

unordered_map<string,Document> tokenizeFiles (string sourceDir, string destDir, WordFilter wordFilter);
void CheckLDAPerformance(int numberOfDocuments, bool debug);

int main(int argc, char* argv[]) {
    // fixing seed for testing purposes
    srand(time(NULL));

    // should be multiple of 3
    int populationSize = 9;
    double fitnessThreshold = 0.85;
    bool metrics = false;
    bool debug = false;
    bool progress = false;

    for (int i = 1; i < argc; i++) {
        string s = argv[i];
        if(s.compare("-p") == 0)
            populationSize = stoi(argv[++i]);
        else if(s.compare("-f") == 0)
            fitnessThreshold = stod(argv[++i]);
        else if(s.compare("-metrics") == 0)
            metrics = true;
        else if(s.compare("-debug") == 0)
            debug = true;
        else if(s.compare("-progress") == 0)
            progress = true;
        else
            cout<<"\tparameter not recognized: "<<argv[i]<<endl;
    }
    cout<<endl;

  clock_t t;
  string stopWordsFile  = "stopwords.txt";
  string ldaInputFile   = "./tempData/input1.txt";
  string dataDir        = "txtData";        // name of the directory that contains the original source data
  string mirrorDir      = "processedData";  // name of the directory where the modified data is to be stored
  string delimiter      = "##lda_delimiter##";

  WordFilter wordFilter(stopWordsFile);

  // tokenize articles
  t = clock();
  unordered_map<string,Document> documentsMap = tokenizeFiles(dataDir, mirrorDir, wordFilter);
  t = clock() - t;
  // Output the time it took to find all article's titles and keywords
  cout << "Preprocessing takes " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms" << endl;

  // write input file for LDA
  ofstream outfile (ldaInputFile);
  for (pair<string, Document> element : documentsMap)
    outfile << element.first << delimiter << (element.second).getKeyWords() << endl;
  outfile.close();

  if(metrics) {
      CheckLDAPerformance(documentsMap.size(), debug);
  }
  else {
      // call genetic logic to perform LDA-GA
      geneticLogic(populationSize, documentsMap.size(), fitnessThreshold, debug, progress);

      // vector<Cluster> clusters = ClusterManager::createClusters();
      // unordered_map<string, Article> articlesMap;
      // unordered_map<string, SourceFile> sourceFileMap;
      //
      // ifstream myfile("input.txt");
      // string line;
      // while(getline (myfile, line, '\n')) {
      //     string filename = line.substr(0, line.find("\tX"));
      //     string keywords = line.substr(line.find("\tX\t")+3);
      //     if(filename.find("$AAA$") != string::npos){
      //         articlesMap[filename] = Article(filename, keywords); // needs real values for testing
      //     }
      //     else{
      //         sourceFileMap[filename] = SourceFile(filename, keywords); // needs real values for testing
      //     }
      // }
      //
      // clusters = ClusterManager::cleanCluster(clusters, articlesMap, sourceFileMap);
      // clusters = ClusterManager::cleanSourceFileCluster(clusters, sourceFileMap);
      //
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
  }
}

void CheckLDAPerformance(int numberOfDocuments, bool debug) {
    int TEST_COUNT = 3;
    long LDATotTime = 0;
    string line;
    int tpcs[] = {20};


    for (int i = 0; i <= 3; i++) {
        int number_of_topics = tpcs[i];
		for (int number_of_iterations = 100; number_of_iterations <= 1000; number_of_iterations +=100) {
            PopulationConfig popCfg;
            popCfg.number_of_topics = number_of_topics;
            popCfg.number_of_iterations = number_of_iterations;
            LDATotTime = 0;

            for (int i = 0; i < TEST_COUNT; ++i) {
                TopicModelling tm(number_of_topics, number_of_iterations, numberOfDocuments, debug);
                string id = "__"+to_string(i/2)+"__"+to_string(number_of_topics)+"x"+to_string(number_of_iterations);

                LDATotTime += tm.LDA(id);
            }
            popCfg.LDA_execution_milliseconds = ((double)LDATotTime/TEST_COUNT);
            cout<< number_of_topics<<"x"<<number_of_iterations<<": " + to_string(popCfg.LDA_execution_milliseconds)<<endl;
        }
    }
}

unordered_map<string,Document> tokenizeFiles (string sourceDir, string destDir, WordFilter wordFilter) {
    DIR* dirp = opendir(sourceDir.c_str());
    struct dirent* entry;
    string filename;
    string semitoken;
    string token;
    string content;
    vector<Document> documentList;
    unordered_map<string,Document> documentsMap;

    while ((entry = readdir(dirp)) != NULL) {
        filename = entry->d_name;

        if(filename.length() > 4 && filename.substr(filename.length()-4)==".txt") {
            content = "";
            ifstream myfile (sourceDir + "/" + filename);

            // TODO: improve tokenization
            while ( getline (myfile,semitoken, '\n') ) {
                stringstream line(semitoken);
                while ( getline (line, token, ' ') ) {
                    token = wordFilter.removeLineMarkers (token);

                    if(wordFilter.isEmpty   (token)) continue;
                    if(wordFilter.isNumber  (token)) continue;
                    if(wordFilter.isStopWord(token)) continue;

                    content += wordFilter.standardizeToLower(token) + " ";
                }
            }

            myfile.close();

            Document newSource(filename, content);
            documentList.push_back(newSource);
            documentsMap[filename] = newSource;

            // Write content to the file
            if (content.length() != 0) {
                ofstream outfile (destDir + "/" + filename);
                outfile << content;
                outfile.close();
            }
        }
    }
    closedir(dirp);

    return documentsMap;
}
