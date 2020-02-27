

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <cassert>
#include "constants.h"
#include "strtokenizer.h"
#include "utils.h"
#include "dataset.h"
#include "model.h"
#include "CUDASampling.h"
#include "cuda_util.h"
#include <iostream>

using namespace std;
using namespace CUDAUtil;

model::~model() {
    if (p) {
        delete p;
    }

    if (ptrndata) {
        delete ptrndata;
    }

    if (pnewdata) {
        delete pnewdata;
    }

    if (z) {
        for (int m = 0; m < M; m++) {
            if (z[m]) {
                delete z[m];
            }
        }
    }

    if (nw) {
        for (int w = 0; w < V; w++) {
            if (nw[w]) {
                delete nw[w];
            }
        }
    }

    if (nd) {
        for (int m = 0; m < M; m++) {
            if (nd[m]) {
                delete nd[m];
            }
        }
    }

    if (nwsum) {
        delete nwsum;
    }

    if (ndsum) {
        delete ndsum;
    }

    if (theta) {
        for (int m = 0; m < M; m++) {
            if (theta[m]) {
                delete theta[m];
            }
        }
    }

    if (phi) {
        for (int k = 0; k < K; k++) {
            if (phi[k]) {
                delete phi[k];
            }
        }
    }

    // only for inference
    if (newz) {
        for (int m = 0; m < newM; m++) {
            if (newz[m]) {
                delete newz[m];
            }
        }
    }

    if (newnw) {
        for (int w = 0; w < newV; w++) {
            if (newnw[w]) {
                delete newnw[w];
            }
        }
    }

    if (newnd) {
        for (int m = 0; m < newM; m++) {
            if (newnd[m]) {
                delete newnd[m];
            }
        }
    }

    if (newnwsum) {
        delete newnwsum;
    }

    if (newndsum) {
        delete newndsum;
    }

    if (newtheta) {
        for (int m = 0; m < newM; m++) {
            if (newtheta[m]) {
                delete newtheta[m];
            }
        }
    }

    if (newphi) {
        for (int k = 0; k < K; k++) {
            if (newphi[k]) {
                delete newphi[k];
            }
        }
    }
}

void model::set_default_values() {
    wordmapfile = "wordmap.txt";
    trainlogfile = "trainlog.txt";
    tassign_suffix = ".tassign";
    theta_suffix = ".theta";
    phi_suffix = ".phi";
    others_suffix = ".others";
    twords_suffix = ".twords";

    dir = "./results/";
    dfile = "trndocs.dat";
    model_name = "model-final";
    model_status = MODEL_STATUS_UNKNOWN;

    ptrndata = NULL;
    pnewdata = NULL;

    M = 0;
    V = 0;
    K = 100;
    alpha = 50.0 / K;
    beta = 0.1;
    niters = 2000;
    liter = 0;
    savestep = 0;
    twords = 0;
    withrawstrs = 0;

    p = NULL;
    z = NULL;
    nw = NULL;
    nd = NULL;
    nwsum = NULL;
    ndsum = NULL;
    theta = NULL;
    phi = NULL;

    newM = 0;
    newV = 0;
    newz = NULL;
    newnw = NULL;
    newnd = NULL;
    newnwsum = NULL;
    newndsum = NULL;
    newtheta = NULL;
    newphi = NULL;
}

int model::parse_args(int argc, char ** argv) {
    return utils::parse_args(argc, argv, this);
}

int model::init(int argc, char ** argv, ConfigOptions* cfg) {
    // call parse_args
    this->cfg = cfg;

    if (parse_args(argc, argv)) {
        return 1;
    }

    if (model_status == MODEL_STATUS_EST) {
        // estimating the model from scratch
        if (init_est()) {
            return 1;
        }

    } else if (model_status == MODEL_STATUS_ESTC) {
        // estimating the model from a previously estimated one
        if (init_estc()) {
            return 1;
        }

    } else if (model_status == MODEL_STATUS_INF) {
        // do inference
        if (init_inf()) {
            return 1;
        }
    }

    return 0;
}

int model::load_model(string model_name) {
    int i, j;

    string filename = dir + model_name + tassign_suffix;
    FILE * fin = fopen(filename.c_str(), "r");
    if (!fin) {
        cfg->logger.log(error, "Cannot open file to load model!");
        return 1;
    }

    char buff[BUFF_SIZE_LONG];
    string line;

    // allocate memory for z and ptrndata
    z = new int*[M];
    ptrndata = new dataset(M, cfg);
    ptrndata->V = V;

    for (i = 0; i < M; i++) {
        char * pointer = fgets(buff, BUFF_SIZE_LONG, fin);
        if (!pointer) {
            cfg->logger.log(error, "Invalid word-topic assignment file, check the number of docs!");
            return 1;
        }

        line = buff;
        strtokenizer strtok(line, " \t\r\n");
        int length = strtok.count_tokens();

        vector<int> words;
        vector<int> topics;
        for (j = 0; j < length; j++) {
            string token = strtok.token(j);

            strtokenizer tok(token, ":");
            if (tok.count_tokens() != 2) {
                cfg->logger.log(error, "Invalid word-topic assignment line!");
                return 1;
            }

            words.push_back(atoi(tok.token(0).c_str()));
            topics.push_back(atoi(tok.token(1).c_str()));
        }

        // allocate and add new document to the corpus
        document * pdoc = new document(words);
        ptrndata->add_doc(pdoc, i);

        // assign values for z
        z[i] = new int[topics.size()];
        for (j = 0; j < topics.size(); j++) {
            z[i][j] = topics[j];
        }
    }

    fclose(fin);

    return 0;
}

int model::save_model(string model_name) {
    if (save_model_tassign(dir + model_name + tassign_suffix)) {
        return 1;
    }

    if (save_model_others(dir + model_name + others_suffix)) {
        return 1;
    }

    if (save_model_theta(dir + model_name + theta_suffix)) {
        return 1;
    }

    if (save_model_phi(dir + model_name + phi_suffix)) {
        return 1;
    }

    if (twords > 0) {
        if (save_model_twords(dir + model_name + twords_suffix)) {
            return 1;
        }
    }

    return 0;
}

int model::save_model_tassign(string filename) {
    int i, j;

    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open assignments file to save!");
        return 1;
    }

    // wirte docs with topic assignments for words
    for (i = 0; i < ptrndata->M; i++) {
        for (j = 0; j < ptrndata->docs[i]->length; j++) {
            fprintf(fout, "%d:%d ", ptrndata->docs[i]->words[j], z[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);

    return 0;
}

int model::save_model_theta(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open theta file to save!");
        return 1;
    }

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) {
            fprintf(fout, "%f ", theta[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);

    return 0;
}

int model::save_model_phi(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open phi file to save!");
        return 1;
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < V; j++) {
            fprintf(fout, "%f ", phi[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);

    return 0;
}

double model::getDistribution(int doc, int topic){
    if(doc<M && topic<K)
        return theta[doc][topic];

    return -1;
}
string model::getDocName(int doc){
    return ptrndata->get_doc_name(doc);
}

vector<pair<int, double> > model::getDocDistributions(int doc){
    vector<pair<int, double> > topics_probs;
    pair<int, double> topic_prob;
    for (int k = 0; k < K; k++) {
        topic_prob.first = k;
        topic_prob.second = theta[doc][k];
        topics_probs.push_back(topic_prob);
    }

        // quick sort to sort word-topic probability
    utils::quicksort(topics_probs, 0, topics_probs.size() - 1);

    return topics_probs;
}
double model::getTopicDistribution(int topic){
    if(topic<K){
        float dist = 0;
        for(int d = 0; d<M; d++){
            dist += theta[d][topic];
        }
        dist /= M;
        return dist;
    }

    return -1;
}

int model::save_model_others(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open others-file to save!");
        return 1;
    }

    fprintf(fout, "alpha=%f\n", alpha);
    fprintf(fout, "beta=%f\n", beta);
    fprintf(fout, "ntopics=%d\n", K);
    fprintf(fout, "ndocs=%d\n", M);
    fprintf(fout, "nwords=%d\n", V);
    fprintf(fout, "liter=%d\n", liter);

    fclose(fout);

    return 0;
}

int model::save_model_twords(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open word-topic file to save!");
        return 1;
    }

    if (twords > V) {
        twords = V;
    }
    mapid2word::iterator it;
    string topicTwords;

    for (int k = 0; k < K; k++) {
        vector<pair<int, double> > words_probs;
        pair<int, double> word_prob;
        for (int w = 0; w < V; w++) {
            word_prob.first = w;
            word_prob.second = phi[k][w];
            words_probs.push_back(word_prob);
        }

        // quick sort to sort word-topic probability
        utils::quicksort(words_probs, 0, words_probs.size() - 1);

        fprintf(fout, "Topic %dth:\n", k);
        topicTwords = "";
        for (int i = 0; i < twords; i++) {
            it = id2word.find(words_probs[i].first);
            if (it != id2word.end()) {
                fprintf(fout, "\t%s   %f\n", (it->second).c_str(), words_probs[i].second);
                topicTwords += it->second + ((i<twords-1) ? " " : "");
            }
        }
        maptopic2Words.insert(pair<int, string>(k, topicTwords));
    }

    fclose(fout);

    return 0;
}

int model::save_inf_model(string model_name) {
    if (save_inf_model_tassign(dir + model_name + tassign_suffix)) {
        return 1;
    }

    if (save_inf_model_others(dir + model_name + others_suffix)) {
        return 1;
    }

    if (save_inf_model_newtheta(dir + model_name + theta_suffix)) {
        return 1;
    }

    if (save_inf_model_newphi(dir + model_name + phi_suffix)) {
        return 1;
    }

    if (twords > 0) {
        if (save_inf_model_twords(dir + model_name + twords_suffix)) {
            return 1;
        }
    }

    return 0;
}

int model::save_inf_model_tassign(string filename) {
    int i, j;

    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open assignments file to save!");
        return 1;
    }

    // wirte docs with topic assignments for words
    for (i = 0; i < pnewdata->M; i++) {
        for (j = 0; j < pnewdata->docs[i]->length; j++) {
            fprintf(fout, "%d:%d ", pnewdata->docs[i]->words[j], newz[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);

    return 0;
}

int model::save_inf_model_newtheta(string filename) {
    int i, j;

    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open new theta file to save!");
        return 1;
    }

    for (i = 0; i < newM; i++) {
        for (j = 0; j < K; j++) {
            fprintf(fout, "%f ", newtheta[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);

    return 0;
}

int model::save_inf_model_newphi(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open new phi file to save!");
        return 1;
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < newV; j++) {
            fprintf(fout, "%f ", newphi[i][j]);
        }
        fprintf(fout, "\n");
    }

    fclose(fout);

    return 0;
}

int model::save_inf_model_others(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open others-file to save!");
        return 1;
    }

    fprintf(fout, "alpha=%f\n", alpha);
    fprintf(fout, "beta=%f\n", beta);
    fprintf(fout, "ntopics=%d\n", K);
    fprintf(fout, "ndocs=%d\n", newM);
    fprintf(fout, "nwords=%d\n", newV);
    fprintf(fout, "liter=%d\n", inf_liter);

    fclose(fout);

    return 0;
}

int model::save_inf_model_twords(string filename) {
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
        cfg->logger.log(error, "Cannot open twords file to save!");
        return 1;
    }

    if (twords > newV) {
        twords = newV;
    }
    mapid2word::iterator it;
    map<int, int>::iterator _it;

    for (int k = 0; k < K; k++) {
        vector<pair<int, double> > words_probs;
        pair<int, double> word_prob;
        for (int w = 0; w < newV; w++) {
            word_prob.first = w;
            word_prob.second = newphi[k][w];
            words_probs.push_back(word_prob);
        }

        // quick sort to sort word-topic probability
        utils::quicksort(words_probs, 0, words_probs.size() - 1);

        fprintf(fout, "Topic %dth:\n", k);
        for (int i = 0; i < twords; i++) {
            _it = pnewdata->_id2id.find(words_probs[i].first);
            if (_it == pnewdata->_id2id.end()) {
                continue;
            }
            it = id2word.find(_it->second);
            if (it != id2word.end()) {
                fprintf(fout, "\t%s   %f\n", (it->second).c_str(), words_probs[i].second);
            }
        }
    }

    fclose(fout);

    return 0;
}

int model::init_est() {
    int m, n, w, k;

    p = new double[K];

    // + read training data
    ptrndata = new dataset(cfg);
    if (ptrndata->read_trndata(dfile, dir + wordmapfile, ndocs)) {
        cfg->logger.log(error, "Fail to read training data!");
        return 1;
    }

    // + allocate memory and assign values for variables
    M = ptrndata->M;
    V = ptrndata->V;
    // K: from command line or default value
    // alpha, beta: from command line or default values
    // niters, savestep: from command line or default values

    nw = new int*[V];
    for (w = 0; w < V; w++) {
        nw[w] = new int[K];
        for (k = 0; k < K; k++) {
            nw[w][k] = 0;
        }
    }

    nd = new int*[M];
    for (m = 0; m < M; m++) {
        nd[m] = new int[K];
        for (k = 0; k < K; k++) {
            nd[m][k] = 0;
        }
    }

    nwsum = new int[K];
    for (k = 0; k < K; k++) {
        nwsum[k] = 0;
    }

    ndsum = new int[M];
    for (m = 0; m < M; m++) {
        ndsum[m] = 0;
    }

    //srandom(time(0)); // initialize for random number generation
    z = new int*[M];
    for (m = 0; m < ptrndata->M; m++) {
        int N = ptrndata->docs[m]->length;
        z[m] = new int[N];

        // initialize for z
        for (n = 0; n < N; n++) {
            int topic = (int) (((double) random() / RAND_MAX) * K);
#if DEBUG_LEVEL == DEBUG_ALL
            assert(topic >= 0 && topic < K);
#endif

            z[m][n] = topic;

            // number of instances of word i assigned to topic j
            nw[ptrndata->docs[m]->words[n]][topic] += 1;
            // number of words in document i assigned to topic j
            nd[m][topic] += 1;
            // total number of words assigned to topic j
            nwsum[topic] += 1;
        }
        // total number of words in document i
        ndsum[m] = N;
    }

    theta = new double*[M];
    for (m = 0; m < M; m++) {
        theta[m] = new double[K];
    }

    phi = new double*[K];
    for (k = 0; k < K; k++) {
        phi[k] = new double[V];
    }

    return 0;
}

int model::init_estc() {
    // estimating the model from a previously estimated one
    int m, n, w, k;

    p = new double[K];

    // load moel, i.e., read z and ptrndata
    if (load_model(model_name)) {
        cfg->logger.log(error, "Fail to load word-topic assignmetn file of the model!");
        return 1;
    }

    nw = new int*[V];
    for (w = 0; w < V; w++) {
        nw[w] = new int[K];
        for (k = 0; k < K; k++) {
            nw[w][k] = 0;
        }
    }

    nd = new int*[M];
    for (m = 0; m < M; m++) {
        nd[m] = new int[K];
        for (k = 0; k < K; k++) {
            nd[m][k] = 0;
        }
    }

    nwsum = new int[K];
    for (k = 0; k < K; k++) {
        nwsum[k] = 0;
    }

    ndsum = new int[M];
    for (m = 0; m < M; m++) {
        ndsum[m] = 0;
    }

    for (m = 0; m < ptrndata->M; m++) {
        int N = ptrndata->docs[m]->length;

        // assign values for nw, nd, nwsum, and ndsum
        for (n = 0; n < N; n++) {
            int w = ptrndata->docs[m]->words[n];
            int topic = z[m][n];

            // number of instances of word i assigned to topic j
            nw[w][topic] += 1;
            // number of words in document i assigned to topic j
            nd[m][topic] += 1;
            // total number of words assigned to topic j
            nwsum[topic] += 1;
        }
        // total number of words in document i
        ndsum[m] = N;
    }

    theta = new double*[M];
    for (m = 0; m < M; m++) {
        theta[m] = new double[K];
    }

    phi = new double*[K];
    for (k = 0; k < K; k++) {
        phi[k] = new double[V];
    }

    return 0;
}

void model::cuda_estimate() {
    if (twords > 0) {
        // print out top words per topic
        ptrndata->read_wordmap(dir + wordmapfile, &id2word);
    }

    cfg->logger.log(debug, "Sampling "+std::to_string(niters)+" iterations (CUDA)!");

    //cout << "#doc: " << M << endl;

    int numBlock = 1024;
    const unsigned int device = 0;
    CUDATimer timer;
    cfg->logger.log(debug, "tnumBlock: " + std::to_string(numBlock) + ", device: " +std::to_string(device));


    //GoldSampling sample(*this, numBlock);
    CUDASampling sample(*this, numBlock, cfg, device);
    cfg->logger.log(debug, "Finished sampling");

    int last_iter = liter;
    for (liter = last_iter + 1; liter <= niters + last_iter; liter++) {
//        cfg->logger.log(debug, "Iteration "+std::to_string(liter)+" ...");

        timer.reset();
        timer.go();
        sample.run();
        timer.stop();
        //cout << "\tTime: " << timer.report() << endl;


        if (savestep > 0) {
            if (liter % savestep == 0) {
                // saving the model
                cfg->logger.log(debug, "Saving the model at iteration "+std::to_string(liter)+" ...");
                sample.copyBack();
                compute_theta();
                compute_phi();
                save_model(utils::generate_model_name(liter));
            }
        }
    }


    //printf("Gibbs sampling completed!\n");
    //printf("Saving the final model!\n");
    sample.copyBack();
    compute_theta();
    compute_phi();
    liter--;
    save_model(utils::generate_model_name(-1));

}

void model::estimate() {
    if (twords > 0) {
        // print out top words per topic
        ptrndata->read_wordmap(dir + wordmapfile, &id2word);
    }

    //printf("Sampling %d iterations (CPU, serial)!\n", niters);

    CPUTimer timer;

    int last_iter = liter;
    for (liter = last_iter + 1; liter <= niters + last_iter; liter++) {
        //printf("Iteration %d ...\n", liter);

        timer.reset();
        timer.go();
        // for all z_i
        for (int m = 0; m < M; m++) {
            for (int n = 0; n < ptrndata->docs[m]->length; n++) {
                // (z_i = z[m][n])
                // sample from p(z_i|z_-i, w)
                int topic = sampling(m, n);
                z[m][n] = topic;
            }
        }
        timer.stop();
        //cout << "\tTime: " << timer.report() << endl;

        if (savestep > 0) {
            if (liter % savestep == 0) {
                // saving the model
                cfg->logger.log(debug, "Saving the model at iteration " + std::to_string(liter) + " ...");
                compute_theta();
                compute_phi();
                save_model(utils::generate_model_name(liter));
            }
        }
    }

    //printf("Gibbs sampling completed!\n");
    //printf("Saving the final model!\n");
    compute_theta();
    compute_phi();
    liter--;
    save_model(utils::generate_model_name(-1));
}

int model::sampling(int m, int n) {
    // remove z_i from the count variables
    int topic = z[m][n];
    int w = ptrndata->docs[m]->words[n];
    nw[w][topic] -= 1;
    nd[m][topic] -= 1;
    nwsum[topic] -= 1;
    ndsum[m] -= 1;

    double Vbeta = V * beta;
    double Kalpha = K * alpha;
    // do multinomial sampling via cumulative method
    for (int k = 0; k < K; k++) {
        p[k] = (nw[w][k] + beta) / (nwsum[k] + Vbeta) *
                (nd[m][k] + alpha) / (ndsum[m] + Kalpha);
    }
    // cumulate multinomial parameters
    for (int k = 1; k < K; k++) {
        p[k] += p[k - 1];
    }
    // scaled sample because of unnormalized p[]
    double u = ((double) random() / RAND_MAX) * p[K - 1];

    for (topic = 0; topic < K; topic++) {
        if (p[topic] >= u) {
            break;
        }
    }

    // add newly estimated z_i to count variables
    nw[w][topic] += 1;
    nd[m][topic] += 1;
    nwsum[topic] += 1;
    ndsum[m] += 1;

    return topic;
}

void model::compute_theta() {
    for (int m = 0; m < M; m++) {
        for (int k = 0; k < K; k++) {
            theta[m][k] = (nd[m][k] + alpha) / (ndsum[m] + K * alpha);
        }
    }
}

void model::compute_phi() {
    for (int k = 0; k < K; k++) {
        for (int w = 0; w < V; w++) {
            phi[k][w] = (nw[w][k] + beta) / (nwsum[k] + V * beta);
        }
    }
}

int model::init_inf() {
    // estimating the model from a previously estimated one
    int m, n, w, k;

    p = new double[K];

    // load moel, i.e., read z and ptrndata
    if (load_model(model_name)) {
        cfg->logger.log(error, "Fail to load word-topic assignmetn file of the model!");
        return 1;
    }

    nw = new int*[V];
    for (w = 0; w < V; w++) {
        nw[w] = new int[K];
        for (k = 0; k < K; k++) {
            nw[w][k] = 0;
        }
    }

    nd = new int*[M];
    for (m = 0; m < M; m++) {
        nd[m] = new int[K];
        for (k = 0; k < K; k++) {
            nd[m][k] = 0;
        }
    }

    nwsum = new int[K];
    for (k = 0; k < K; k++) {
        nwsum[k] = 0;
    }

    ndsum = new int[M];
    for (m = 0; m < M; m++) {
        ndsum[m] = 0;
    }

    for (m = 0; m < ptrndata->M; m++) {
        int N = ptrndata->docs[m]->length;

        // assign values for nw, nd, nwsum, and ndsum
        for (n = 0; n < N; n++) {
            int w = ptrndata->docs[m]->words[n];
            int topic = z[m][n];

            // number of instances of word i assigned to topic j
            nw[w][topic] += 1;
            // number of words in document i assigned to topic j
            nd[m][topic] += 1;
            // total number of words assigned to topic j
            nwsum[topic] += 1;
        }
        // total number of words in document i
        ndsum[m] = N;
    }

    // read new data for inference
    pnewdata = new dataset(cfg);
    if (withrawstrs) {
        if (pnewdata->read_newdata_withrawstrs(dfile, dir + wordmapfile)) {
            cfg->logger.log(error, "Fail to read new data!");
            return 1;
        }
    } else {
        if (pnewdata->read_newdata(dfile, dir + wordmapfile)) {
            cfg->logger.log(error, "Fail to read new data!");
            return 1;
        }
    }

    newM = pnewdata->M;
    newV = pnewdata->V;

    newnw = new int*[newV];
    for (w = 0; w < newV; w++) {
        newnw[w] = new int[K];
        for (k = 0; k < K; k++) {
            newnw[w][k] = 0;
        }
    }

    newnd = new int*[newM];
    for (m = 0; m < newM; m++) {
        newnd[m] = new int[K];
        for (k = 0; k < K; k++) {
            newnd[m][k] = 0;
        }
    }

    newnwsum = new int[K];
    for (k = 0; k < K; k++) {
        newnwsum[k] = 0;
    }

    newndsum = new int[newM];
    for (m = 0; m < newM; m++) {
        newndsum[m] = 0;
    }

    //srandom(time(0)); // initialize for random number generation
    newz = new int*[newM];
    for (m = 0; m < pnewdata->M; m++) {
        int N = pnewdata->docs[m]->length;
        newz[m] = new int[N];

        // assign values for nw, nd, nwsum, and ndsum
        for (n = 0; n < N; n++) {
            int w = pnewdata->docs[m]->words[n];
            int _w = pnewdata->_docs[m]->words[n];
            int topic = (int) (((double) random() / RAND_MAX) * K);
            assert(topic >= 0 && topic < K);
            newz[m][n] = topic;

            // number of instances of word i assigned to topic j
            newnw[_w][topic] += 1;
            // number of words in document i assigned to topic j
            newnd[m][topic] += 1;
            // total number of words assigned to topic j
            newnwsum[topic] += 1;
        }
        // total number of words in document i
        newndsum[m] = N;
    }

    newtheta = new double*[newM];
    for (m = 0; m < newM; m++) {
        newtheta[m] = new double[K];
    }

    newphi = new double*[K];
    for (k = 0; k < K; k++) {
        newphi[k] = new double[newV];
    }

    return 0;
}

void model::inference() {
    if (twords > 0) {
        // print out top words per topic
        ptrndata->read_wordmap(dir + wordmapfile, &id2word);
    }

    cfg->logger.log(debug, "Sampling " + std::to_string(niters) + " iterations for inference!");

    for (inf_liter = 1; inf_liter <= niters; inf_liter++) {
        cfg->logger.log(info, "Iteration " + std::to_string(inf_liter) + " ...");

        // for all newz_i
        for (int m = 0; m < newM; m++) {
            for (int n = 0; n < pnewdata->docs[m]->length; n++) {
                // (newz_i = newz[m][n])
                // sample from p(z_i|z_-i, w)
                int topic = inf_sampling(m, n);
                newz[m][n] = topic;
            }
        }
    }

    cfg->logger.log(debug, "Gibbs sampling for inference completed!\n");
    cfg->logger.log(debug, "Saving the inference outputs!\n");
    compute_newtheta();
    compute_newphi();
    inf_liter--;
    save_inf_model(dfile);


    compute_and_save_perplexity("perplexity.txt");
}

int model::compute_and_save_perplexity(string filename) {
    double perplexity = compute_perplexity();
    FILE * fout = fopen(filename.c_str(), "a");
    if (!fout) {
        cfg->logger.log(error, "Cannot open perplexity file to save!");
        return 1;
    }

    fprintf(fout, "[%s][%s]\n", this->dir.c_str(), this->model_name.c_str());
    fprintf(fout, "K=%d\n", K);
    fprintf(fout, "perplexity=%f\n", perplexity);
    fprintf(fout, "\n");
    fclose(fout);
    return 0;
}

double model::compute_perplexity() {
    double perplexity = 0.0;
    double denominator = 0.0;
    double numerator = 0.0;
    double perp = 0.0;

    cfg->logger.log(info, "newM =" + std::to_string(newM));
    for (int m = 0; m < newM; m++) {
        int Nd = pnewdata->docs[m]->length;

        if (Nd > 0) {
            perp = 0.0;
            double p = 0.0;
            for (int n = 0; n < Nd; n++) {
                int w = pnewdata->_docs[m]->words[n];
                for (int k = 0; k < K; k++) {
                    //p += newphi[k][w];
                    p += newphi[k][w] * newtheta[m][k];
                }
                if (p == 0)
                    cfg->logger.log(info, "fds");
                if (::isnan(p))
                    cfg->logger.log(info, "nan ");
                perp += log(p);
            }
            numerator = perp;
            denominator = Nd;
            perplexity += exp(-numerator / denominator);
        }
    }

    //perplexity = exp(-numerator/denominator);
    return perplexity;
}

int model::inf_sampling(int m, int n) {
    // remove z_i from the count variables
    int topic = newz[m][n];
    int w = pnewdata->docs[m]->words[n];
    int _w = pnewdata->_docs[m]->words[n];
    newnw[_w][topic] -= 1;
    newnd[m][topic] -= 1;
    newnwsum[topic] -= 1;
    newndsum[m] -= 1;

    double Vbeta = V * beta;
    double Kalpha = K * alpha;
    // do multinomial sampling via cumulative method
    for (int k = 0; k < K; k++) {
        p[k] = (nw[w][k] + newnw[_w][k] + beta) / (nwsum[k] + newnwsum[k] + Vbeta) *
                (newnd[m][k] + alpha) / (newndsum[m] + Kalpha);
    }
    // cumulate multinomial parameters
    for (int k = 1; k < K; k++) {
        p[k] += p[k - 1];
    }
    // scaled sample because of unnormalized p[]
    double u = ((double) random() / RAND_MAX) * p[K - 1];

    for (topic = 0; topic < K; topic++) {
        if (p[topic] >= u) {
            break;
        }
    }

    // add newly estimated z_i to count variables
    newnw[_w][topic] += 1;
    newnd[m][topic] += 1;
    newnwsum[topic] += 1;
    newndsum[m] += 1;

    return topic;
}

void model::compute_newtheta() {
    for (int m = 0; m < newM; m++) {
        for (int k = 0; k < K; k++) {
            newtheta[m][k] = (newnd[m][k] + alpha) / (newndsum[m] + K * alpha);
        }
    }
}

void model::compute_newphi() {
    map<int, int>::iterator it;
    for (int k = 0; k < K; k++) {
        for (int w = 0; w < newV; w++) {
            it = pnewdata->_id2id.find(w);
            if (it != pnewdata->_id2id.end()) {
                newphi[k][w] = (nw[it->second][k] + newnw[w][k] + beta) / (nwsum[k] + newnwsum[k] + V * beta);
            }
        }
    }
}
