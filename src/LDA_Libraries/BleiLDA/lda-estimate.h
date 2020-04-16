#ifndef LDA_ESTIMATE_H
#define LDA_ESTIMATE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>

#include "lda.h"
#include "lda-data.h"
#include "lda-model.h"
#include "lda-alpha.h"
#include "utils.h"
#include "lda-inference.h"

class LDA_Estimate {
public:
int LAG = 5;

float EM_CONVERGED;
int EM_MAX_ITER;
int ESTIMATE_ALPHA;
double INITIAL_ALPHA;
int NTOPICS;
int NDOCS;
double** docTopDist;
double* topDist;

LDA_Estimate(int ndocs, int ntopics);
~LDA_Estimate();
double getDocTopDist(int doc, int topic);
double getTopicDist(int topic);
int getMainTopic(int doc);

double doc_e_step(blda_document* doc,
                  double* gamma,
                  double** phi,
                  blda_model* model,
                  blda_suffstats* ss);

void save_gamma(char* filename,
                double** gamma,
                int num_docs,
                int num_topics);

void run_em(char* start,
            char* directory,
            blda_corpus* corpus);
void read_settings(char* filename);
void init_settings(float convergence, int alpha);

void infer(char* model_root,
           char* save,
           blda_corpus* corpus);

int runLDA(blda_corpus* corpus, int ntopics, int iter, float iniAlpha, string outFolder);
};

#endif

