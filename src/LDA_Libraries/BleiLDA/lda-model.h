#ifndef LDA_MODEL_H
#define LDA_MODEL_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "lda.h"
#include "lda-alpha.h"
#include "../../utils.h"

#define NUM_INIT 1

void free_lda_model(blda_model*);
void save_lda_model(blda_model*, char*);
blda_model* new_lda_model(int, int);
blda_suffstats* new_lda_suffstats(blda_model* model);
void corpus_initialize_ss(blda_suffstats* ss, blda_model* model, blda_corpus* c);
void manual_initialize_ss(char *seedfile, blda_suffstats* ss, blda_model* model, blda_corpus* c);
void random_initialize_ss(blda_suffstats* ss, blda_model* model);
void zero_initialize_ss(blda_suffstats* ss, blda_model* model);
void lda_mle(blda_model* model, blda_suffstats* ss, int estimate_alpha);
blda_model* load_lda_model(char* model_root);

#endif
