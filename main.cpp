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

#define POPULATION_SIZE 10

int main() {
    // fixing seed for testing purposes
    srand(1);

  clock_t t;
  string stopWordsFile  = "stopwords.txt";
  string ldaInputFile   = "input1.txt";
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

  // call genetic logic to perform LDA-GA
  geneticLogic(POPULATION_SIZE, documentsMap.size());

  // TODO: call cluster on topics

  // TODO: calculate precision
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
