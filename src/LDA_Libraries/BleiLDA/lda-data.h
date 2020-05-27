#ifndef LDA_DATA_H
#define LDA_DATA_H

#include <stdio.h>
#include <stdlib.h>

#include "lda.h"

#define OFFSET 0;                  // offset for reading data

blda_corpus* read_data(char* data_filename);
int max_corpus_length(blda_corpus* c);

#endif