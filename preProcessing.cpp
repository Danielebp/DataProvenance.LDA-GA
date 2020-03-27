#include "preProcessing.h"

unordered_map<string,Document> tokenizeFiles (string sourceDir, string destDir, WordFilter wordFilter, ConfigOptions* cfg) {
    DIR* dirp = opendir(sourceDir.c_str());
    struct dirent* entry;
    string filename;
    string semitoken;
    string token;
    string content;
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

            if (wordFilter.isEmpty(content)) {
                cfg->logger.log(debug, "Removing document "+filename+". Content is empty.");
            }
            else {
                Document newSource(filename, content);
                documentsMap[filename] = newSource;

                // Write content to the file
                ofstream outfile (destDir + "/" + filename);
                outfile << content;
                outfile.close();
            }
        }
    }
    closedir(dirp);

    return documentsMap;
}

unordered_map<string, Document> loadPreProcessed(ConfigOptions* cfg){
    unordered_map<string,Document> documentsMap;
    ifstream infile (cfg->preProcessedFile);
	cfg->logger.log(debug, "Opened Input File - " + cfg->preProcessedFile);
    ofstream outfile (cfg->ldaInputFile);
        cfg->logger.log(debug, "Created Output File - " + cfg->ldaInputFile);
    string line;

    while ( getline (infile, line, '\n') ) {
        cfg->logger.log(debug, line);
        string filename = line.substr(0, line.find(cfg->delimiter));
        string content = line.substr(line.find(cfg->delimiter)+17);

        if(content.empty()){
            cfg->logger.log(debug, "Removing document "+filename+". Content is empty.");
            continue;
        }

        Document newSource(filename, content);
        documentsMap[filename] = newSource;
        outfile<<filename<<cfg->delimiter<<content<<endl;
    }
        cfg->logger.log(debug, "Done loading files");
    outfile.close();
    infile.close();
    return documentsMap;
}

unordered_map<string, Document> preProcess(ConfigOptions* cfg){
    clock_t t;
    WordFilter wordFilter(cfg->stopWordsFile);
    cfg->logger.log(debug, "Starting to tokenize files");
    // tokenize articles
    t = clock();
    unordered_map<string,Document> documentsMap = tokenizeFiles(cfg->dataDir, cfg->mirrorDir, wordFilter, cfg);
    t = clock() - t;
    // Output the time it took to find all article's titles and keywords
    cfg->logger.log(status, "Preprocessing takes " + std::to_string(((float)t)/(CLOCKS_PER_SEC/1000)) + "ms");

    // write input file for LDA
    ofstream outfile (cfg->ldaInputFile);
    for (pair<string, Document> element : documentsMap)
      outfile << element.first << cfg->delimiter << (element.second).getKeyWords() << endl;
    outfile.close();

    return documentsMap;
}

void createLightLDAFiles(ConfigOptions* cfg, int ndocs, string dir){
	createLibsvmFile(cfg->ldaInputFile, cfg->libsvmFile, cfg->wordmapFile, ndocs, cfg->delimiter) ;

	char* args[] = {
		const_cast<char*>(cfg->libsvmFile.c_str(),
		const_cast<char*>(cfg->wordmapFile.c_str())), 
		const_cast<char*>(dir.c_str()), 
		(char*)"0", NULL};
	createBinaryFile(4, args);
}

