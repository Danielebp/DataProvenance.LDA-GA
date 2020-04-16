#include "lda-estimate.h"

int main(void){

    blda_corpus* ldacorpus = read_data("test.txt");
    LDA_Estimate blda_model(50, 10);

    blda_model.runLDA(ldacorpus, 10, 100, 0.1, "./");

    return 0;
}
