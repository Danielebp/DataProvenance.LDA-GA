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
    cfg->logger.log(status, "Preprocessing takes " + to_string(((float)t)/(CLOCKS_PER_SEC/1000)) + "ms");

    // write input file for LDA
    ofstream outfile (cfg->ldaInputFile);
    for (pair<string, Document> element : documentsMap)
      outfile << element.first << cfg->delimiter << (element.second).getKeyWords() << endl;
    outfile.close();

    return documentsMap;
}

#ifdef USELLDA
void createLightLDAFiles(ConfigOptions* cfg, int ndocs){
        cfg->logger.log(debug, "Create Libsvm Files");
	createLibsvmFile(cfg->ldaInputFile, cfg->libsvmFile, cfg->wordmapFile, cfg->docmapFile, ndocs, cfg->delimiter) ;

        cfg->logger.log(debug, "Create Binary Dump");
	char* args[] = {
		const_cast<char*>(cfg->libsvmFile.c_str()),
		const_cast<char*>(cfg->wordmapFile.c_str()),
		const_cast<char*>(cfg->inputDir.c_str()),
		(char*)"0", NULL};
	createBinaryFile(4, args);
}
#endif

void createBleiLDAFiles(ConfigOptions* cfg){
        cfg->logger.log(debug, "Create BleiLDA Files");

        map<string, int> word2id;
        map<int, int>* wordCountDoc;
        map<int, int>wordCountTot;
        map<string, int>::iterator wordmapIt;

        int docID = 0;
        string line;
        ifstream infile (cfg->ldaInputFile);
        ofstream outLibsvm(cfg->libsvmFile);
        ofstream outDocMap(cfg->docmapFile);

        while ( getline (infile, line, '\n') ) {

            int pos = line.find(cfg->delimiter);
            string fname = line.substr(0, pos);

            if(pos!=string::npos)
                line.erase(0, pos + 17);

            strtokenizer strtok(line, " \t\r\n");
            int length = strtok.count_tokens();

            if (length <= 0) {
                cfg->logger.log(info, "Invalid (empty) document: " + fname);
                continue;
            }

            outDocMap<<docID++<<"\t"<<fname<<endl;

            // start processing each word of this line and adding them to a  map
            wordCountDoc = new map<int, int>();
            for (int token = 0; token < length; token++) {
                wordmapIt = word2id.find(strtok.token(token));
                if (wordmapIt == word2id.end()) {
                // word not found, i.e., new word
                wordCountDoc->insert(pair<int, int>(word2id.size(), 1));
                word2id.insert(pair<string, int>(strtok.token(token), word2id.size()));
                } else {
                    (*wordCountDoc)[wordmapIt->second] = (*wordCountDoc)[wordmapIt->second]+1;
                }
            }

            // write processed line to libsvm file
            outLibsvm<<wordCountDoc->size()<<"\t";
            for (pair<int, int> element : (*wordCountDoc)) {
                outLibsvm<<element.first<<":"<<element.second<<" ";

                // update total count
                if (wordCountTot.find(element.first) == wordCountTot.end()) {
                    // word not found, i.e., new word
                    wordCountTot.insert(pair<int, int>(element.first, element.second));
                } else {
                    wordCountTot[element.first] = wordCountTot[element.first]+element.second;
                }
            }
            outLibsvm<<endl;

            delete wordCountDoc;
        }

        outLibsvm.close();
        outDocMap.close();
        infile.close();

        // write word map
        ofstream outWordMap;
        outWordMap.open (cfg->wordmapFile);
        for (pair<string, int> element : word2id) {
            outWordMap<<element.second<<"\t"<<element.first<<"\t"<<wordCountTot[element.second]<<endl;
        }
        outWordMap.close();

        return;

}

unordered_map<string, Document> prepareData(ConfigOptions* cfg){
    unordered_map<string, Document> documentsMap;
    if(cfg->skipPreprocess){
        cfg->logger.log(debug, "Skipping preprocess using " + cfg->preProcessedFile);
        documentsMap = loadPreProcessed(cfg);
        cfg->logger.log(debug, "Loaded "+std::to_string(documentsMap.size())+" documents");
    }
    else{
       cfg->logger.log(debug, "Starting preprocess");
       documentsMap = preProcess(cfg);
    }

    switch (cfg->ldaLibrary) {
#ifdef USELLDA
         case llda:
            cfg->logger.log(debug, "Creating LightLDA files");
            createLightLDAFiles(cfg, documentsMap.size());
            break;
#endif
         case blda:
            createBleiLDAFiles(cfg);
            break;
         case wlda:
            char* args[] = {(char*)"format",
                       (char*)"-input", (char*)cfg->ldaInputFile.cstr(),
                       (char*)"-prefix", (char*)(cfg->inputDir + "/wlda_input"),
                        NULL};
            wlda_format();
            break;
         default:
           break;
     }

     return documentsMap;
}
