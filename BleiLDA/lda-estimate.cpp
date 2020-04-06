// (C) Copyright 2004, David M. Blei (blei [at] cs [dot] cmu [dot] edu)

// This file is part of LDA-C.

// LDA-C is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your
// option) any later version.

// LDA-C is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA

#include "lda-estimate.h"

/*
 * perform inference on a document and update sufficient statistics
 *
 */


double LDA_Estimate::doc_e_step(blda_document* doc, double* gamma, double** phi,
                  blda_model* model, blda_suffstats* ss)
{
    double likelihood;
    int n, k;

    // posterior inference

    likelihood = lda_inference(doc, model, gamma, phi);

    // update sufficient statistics

    double gamma_sum = 0;
    for (k = 0; k < model->num_topics; k++)
    {
        gamma_sum += gamma[k];
        ss->alpha_suffstats += digamma(gamma[k]);
    }
    ss->alpha_suffstats -= model->num_topics * digamma(gamma_sum);

    for (n = 0; n < doc->length; n++)
    {
        for (k = 0; k < model->num_topics; k++)
        {
            ss->class_word[k][doc->words[n]] += doc->counts[n]*phi[n][k];
            ss->class_total[k] += doc->counts[n]*phi[n][k];
        }
    }

    ss->num_docs = ss->num_docs + 1;

    return(likelihood);
}


/*
 * writes the word assignments line for a document to a file
 *
 */

void write_word_assignment(FILE* f, blda_document* doc, double** phi, blda_model* model)
{
    int n;

    fprintf(f, "%03d", doc->length);
    for (n = 0; n < doc->length; n++)
    {
        fprintf(f, " %04d:%02d",
                doc->words[n], argmax(phi[n], model->num_topics));
    }
    fprintf(f, "\n");
    fflush(f);
}


/*
 * saves the gamma parameters of the current dataset
 *
 */

void LDA_Estimate::save_gamma(char* filename, double** gamma, int num_docs, int num_topics)
{
    FILE* fileptr;
    int d, k;
    fileptr = fopen(filename, "w");

    for (d = 0; d < num_docs; d++)
    {
	fprintf(fileptr, "%5.10f", gamma[d][0]);
	for (k = 1; k < num_topics; k++)
	{
	    fprintf(fileptr, " %5.10f", gamma[d][k]);
	}
	fprintf(fileptr, "\n");
    }
    fclose(fileptr);
}


LDA_Estimate::LDA_Estimate(int ndocs, int ntopics){
    NDOCS = ndocs;
    NTOPICS = ntopics;
    docTopDist = (double**)malloc(sizeof(double*)*(NDOCS));
    for (int d = 0; d < NDOCS; d++)
        docTopDist[d] = (double*)malloc(sizeof(double) * NTOPICS);
    topDist = (double*)malloc(sizeof(double)*NTOPICS);
}

LDA_Estimate::~LDA_Estimate(){
    for (int d = 0; d < NDOCS; d++)
        free(docTopDist[d]);

    free(docTopDist);
    free(topDist);
}

double LDA_Estimate::getDocTopDist(int doc, int topic){
    double total = 0;
    for (int t = 0; t < NTOPICS; t++){
        total += docTopDist[doc][t];
    }

    return (docTopDist[doc][topic]/total);
}

int LDA_Estimate::getMainTopic(int doc){
    int maxID = -1;
    int maxDist = -1;
    for (int t = 0; t < NTOPICS; t++){
        if(docTopDist[doc][t]>maxDist){
            maxID = t;
            maxDist = docTopDist[doc][t];
        }
    }

    return maxID;
}

double LDA_Estimate::getTopicDist(int topic){
    double total = 0;
    for (int t = 0; t < NTOPICS; t++){
        total += topDist[t];
    }

    return (topDist[topic]/total);
}

/*
 * run_em
 *
 */

void LDA_Estimate::run_em(char* start, char* directory, blda_corpus* corpus)
{
    int d, n;
    blda_model *model = NULL;
    double **phi;

    // allocate variational parameters

    int max_length = max_corpus_length(corpus);
    phi = (double**)malloc(sizeof(double*)*max_length);
    for (n = 0; n < max_length; n++)
	      phi[n] = (double*)malloc(sizeof(double) * NTOPICS);

    // initialize model

    char filename[100];

    blda_suffstats* ss = NULL;
    if (strcmp(start, "seeded")==0)
    {
        model = new_lda_model(corpus->num_terms, NTOPICS);
        ss = new_lda_suffstats(model);
        corpus_initialize_ss(ss, model, corpus);
        lda_mle(model, ss, 0);
        model->alpha = INITIAL_ALPHA;
    }
    else if (strcmp(start, "random")==0)
    {
        model = new_lda_model(corpus->num_terms, NTOPICS);
        ss = new_lda_suffstats(model);
        random_initialize_ss(ss, model);
        lda_mle(model, ss, 0);
        model->alpha = INITIAL_ALPHA;
    }
    else if (strncmp(start, "manual=",7)==0)
    {
        model = new_lda_model(corpus->num_terms, NTOPICS);
        ss = new_lda_suffstats(model);
        manual_initialize_ss(start + 7, ss, model, corpus);
        lda_mle(model, ss, 0);
        model->alpha = INITIAL_ALPHA;
    }
    else
    {
        model = load_lda_model(start);
        ss = new_lda_suffstats(model);
    }

    sprintf(filename,"%s/000",directory);
    save_lda_model(model, filename);

    // run expectation maximization

    int i = 0;
    double likelihood, likelihood_old = 0, converged = 1;
    sprintf(filename, "%s/likelihood.dat", directory);
    FILE* likelihood_file = fopen(filename, "w");


    while (((converged < 0) || (converged > EM_CONVERGED) || (i <= 2)) && (i <= EM_MAX_ITER))
    {
        i++;
        likelihood = 0;
        zero_initialize_ss(ss, model);

        // e-step

        for (d = 0; d < corpus->num_docs; d++)
        {
            likelihood += doc_e_step(&(corpus->docs[d]),
                                     docTopDist[d],
                                     phi,
                                     model,
                                     ss);
        }

        // m-step

        lda_mle(model, ss, ESTIMATE_ALPHA);

        // check for convergence

        converged = (likelihood_old - likelihood) / (likelihood_old);
        if (converged < 0) VAR_MAX_ITER = VAR_MAX_ITER * 2;
        likelihood_old = likelihood;

        // output model and likelihood

        fprintf(likelihood_file, "%10.10f\t%5.5e\n", likelihood, converged);
        fflush(likelihood_file);
        if ((i % LAG) == 0)
        {
            sprintf(filename,"%s/%03d",directory, i);
            save_lda_model(model, filename);
            sprintf(filename,"%s/%03d.gamma",directory, i);
            save_gamma(filename, docTopDist, corpus->num_docs, model->num_topics);
        }
    }

    // output the final model

    sprintf(filename,"%s/final",directory);
    save_lda_model(model, filename);
    sprintf(filename,"%s/final.gamma",directory);
    save_gamma(filename, docTopDist, corpus->num_docs, model->num_topics);

    // copies topic distribution
    // TODO: investigate this, class_total might not be the distribution
    for(int t=0; t<NTOPICS; t++){
        topDist[t] = ss->class_total[t];
    }

    // output the word assignments (for visualization)

    sprintf(filename, "%s/word-assignments.dat", directory);
    FILE* w_asgn_file = fopen(filename, "w");
    for (d = 0; d < corpus->num_docs; d++)
    {
        likelihood += lda_inference(&(corpus->docs[d]), model, docTopDist[d], phi);
        write_word_assignment(w_asgn_file, &(corpus->docs[d]), phi, model);
    }
    fclose(w_asgn_file);
    fclose(likelihood_file);
}


/*
 * read settings.
 *
 */

void LDA_Estimate::read_settings(char* filename)
{
    FILE* fileptr;
    char alpha_action[100];
    fileptr = fopen(filename, "r");
    fscanf(fileptr, "var max iter %d\n", &VAR_MAX_ITER);
    fscanf(fileptr, "var convergence %f\n", &VAR_CONVERGED);
    fscanf(fileptr, "em max iter %d\n", &EM_MAX_ITER);
    fscanf(fileptr, "em convergence %f\n", &EM_CONVERGED);
    fscanf(fileptr, "alpha %s", alpha_action);
    if (strcmp(alpha_action, "fixed")==0)
    {
	ESTIMATE_ALPHA = 0;
    }
    else
    {
	ESTIMATE_ALPHA = 1;
    }
    fclose(fileptr);
}


void LDA_Estimate::init_settings(float convergence, int alpha)
{
    EM_CONVERGED = convergence;
    ESTIMATE_ALPHA = alpha;
    VAR_CONVERGED = 0.000001;
    VAR_MAX_ITER = 20;
}


/*
 * update sufficient statistics
 *
 */


 int LDA_Estimate::runLDA(blda_corpus* corpus, int ntopics, int iter, float iniAlpha, string outFolder)
 {
     init_settings(0.0001, 1);

     EM_MAX_ITER = iter;
     INITIAL_ALPHA = iniAlpha;
     NTOPICS = ntopics;
     char* dir = const_cast<char*>(outFolder.c_str());
     run_em("random", dir, corpus);

     return(0);
 }
