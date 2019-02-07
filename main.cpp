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
using namespace std;

bool isStopWord(string s, unordered_map<string,long> stopwords);
unordered_map<string,long> loadStopWords (string filename);
unordered_map<string,Document> tokenizeFiles (string sourceDir, string destDir, unordered_map<string,long> stopWords);

int main() 
{
    clock_t t;
    string stopWordsFile = "stopwords.txt";
    unordered_map<string,long> stopWords = loadStopWords(stopWordsFile);
    
    string dataDir =  "txtData";		// name of the directory that contains the original source data
    string mirrorDir =  "processedData";		//name of the directory where the modified data is to be stored

    // starts timer for performance measurement
    t = clock();
    
	// tokenize articles
    unordered_map<string,Document> documentsMap = tokenizeFiles(dataDir, mirrorDir, stopWords);

    // Output the time it took to find all article's titles and keywords
    t = clock() - t;
    cout << "Preprocessing takes " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms" << endl;
        
	
	// TODO: call genetic logic to perform LDA-GA
	
	// TODO: call cluster on topics
	
	// TODO: calculate precision
}


unordered_map<string,long> loadStopWords (string filename) 
{
    unordered_map<string,long> stopWords;
    string line;
    ifstream myfile (filename);
    
    while ( getline (myfile,line) )
    {
        line = trim(line);
        stopWords[line] = 0;
    }
    myfile.close();
    
    return stopWords;
}


unordered_map<string,Document> tokenizeFiles (string sourceDir, string destDir, unordered_map<string,long> stopWords)
{
    DIR* dirp = opendir(sourceDir.c_str());
    struct dirent* entry;
    struct stat *buf;
    string filename;
    string semitoken;
    string token;
    string content;
    vector<Document> documentList;
    unordered_map<string,Document> documentsMap;
    

    while ((entry = readdir(dirp)) != NULL){
        filename = entry->d_name;
        
        if(filename.length() > 4 && filename.substr(filename.length()-4)==".txt")
        {
            content = "";
            cout<<filename<<endl;
            filename = sourceDir + "/" + filename;
            ifstream myfile (filename);
    
            while ( getline (myfile,semitoken, '\n') )
            {
                stringstream line(semitoken);
                while ( getline (line, token, ' ') )
                {
                    if(token[0] == '\r')
                        token = token.substr(1);
                    
                    if(token[token.length()-1] == '\r')
                        token = token.substr(0, token.length()-1);
                    
                    if(token.length()<1) continue;
                    
                    if(isNumber(token)) continue;

                    if(isStopWord(token, stopWords)) continue;

                    content += standardizeToLower(token) + " ";

                }
            }
            myfile.close();

            Document newSource(filename, content);
            documentList.push_back(newSource);
            documentsMap[filename] = newSource;

            // Write content to the file
            if (content.length() != 0) {
                ofstream outfile (destDir + "/" + getFileName(filename));
                outfile << content;
                outfile.close();
            }
        }
    }
    closedir(dirp);

    return documentsMap;
}


bool isStopWord(string s, unordered_map<string,long> stopwords)
{

    if (s != "" && s != " " && stopwords.find(s) == stopwords.end()) 
        return false; 

    return true;
}

