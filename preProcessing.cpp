#include "preProcessing.h"

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


unordered_map<string, Document> preProcess(ConfigOptions* cfg){
    clock_t t;
    WordFilter wordFilter(cfg->stopWordsFile);
    cfg->logger.log(debug, "Starting to tokenize files");
    // tokenize articles
    t = clock();
    unordered_map<string,Document> documentsMap = tokenizeFiles(cfg->dataDir, cfg->mirrorDir, wordFilter);
    t = clock() - t;
    // Output the time it took to find all article's titles and keywords
    cfg->logger.log(info, "Preprocessing takes " + std::to_string(((float)t)/(CLOCKS_PER_SEC/1000)) + "ms");

    // write input file for LDA
    ofstream outfile (cfg->ldaInputFile);
    for (pair<string, Document> element : documentsMap)
      outfile << element.first << cfg->delimiter << (element.second).getKeyWords() << endl;
    outfile.close();

    return documentsMap;
}
