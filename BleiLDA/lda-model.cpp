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

#include "lda-model.h"

/*
 * compute MLE lda model from sufficient statistics
 *
 */

void lda_mle(blda_model* model, blda_suffstats* ss, int estimate_alpha)
{
    int k; int w;

    for (k = 0; k < model->num_topics; k++)
    {
        for (w = 0; w < model->num_terms; w++)
        {
            if (ss->class_word[k][w] > 0)
            {
                model->log_prob_w[k][w] =
                    log(ss->class_word[k][w]) -
                    log(ss->class_total[k]);
            }
            else
                model->log_prob_w[k][w] = -100;
        }
    }
    if (estimate_alpha == 1)
    {
        model->alpha = opt_alpha(ss->alpha_suffstats,
                                 ss->num_docs,
                                 model->num_topics);

    }
}

/*
 * allocate sufficient statistics
 *
 */

blda_suffstats* new_lda_suffstats(blda_model* model)
{
    int num_topics = model->num_topics;
    int num_terms = model->num_terms;
    int i,j;

    blda_suffstats* ss = (blda_suffstats*)malloc(sizeof(blda_suffstats));
    ss->class_total = (double*)malloc(sizeof(double)*num_topics);
    ss->class_word = (double**)malloc(sizeof(double*)*num_topics);
    for (i = 0; i < num_topics; i++)
    {
	ss->class_total[i] = 0;
	ss->class_word[i] = (double*)malloc(sizeof(double)*num_terms);
	for (j = 0; j < num_terms; j++)
	{
	    ss->class_word[i][j] = 0;
	}
    }
    return(ss);
}


/*
 * various intializations for the sufficient statistics
 *
 */

void zero_initialize_ss(blda_suffstats* ss, blda_model* model)
{
    int k, w;
    for (k = 0; k < model->num_topics; k++)
    {
        ss->class_total[k] = 0;
        for (w = 0; w < model->num_terms; w++)
        {
            ss->class_word[k][w] = 0;
        }
    }
    ss->num_docs = 0;
    ss->alpha_suffstats = 0;
}


void random_initialize_ss(blda_suffstats* ss, blda_model* model)
{
    int num_topics = model->num_topics;
    int num_terms = model->num_terms;
    int k, n;
    for (k = 0; k < num_topics; k++)
    {
        for (n = 0; n < num_terms; n++)
        {
            ss->class_word[k][n] += 1.0/num_terms + getRandomFloat();
            ss->class_total[k] += ss->class_word[k][n];
        }
    }
}


void corpus_initialize_ss(blda_suffstats* ss, blda_model* model, blda_corpus* c)
{
    int num_topics = model->num_topics;
    int i, j, k, d, n;
    blda_document* doc;
    int seen[num_topics][NUM_INIT];
    int already_selected;

    for (k = 0; k < num_topics; k++)
    {
        for (i = 0; i < NUM_INIT; i++)
        {
            do
            {
              d = floor(getRandomFloat() * c->num_docs);

              already_selected = 0;
              for (j = 0;j < k;j++)
              {
                if (seen[j][i] == d)
                {
                  already_selected = 1;
                  printf("skipping duplicate seed document %d\n", d);
                }
              }
            } while (already_selected);
            seen[k][i] = d;

            doc = &(c->docs[d]);
            for (n = 0; n < doc->length; n++)
            {
                ss->class_word[k][doc->words[n]] += doc->counts[n];
            }
        }
        for (n = 0; n < model->num_terms; n++)
        {
            ss->class_word[k][n] += 1.0;
            ss->class_total[k] = ss->class_total[k] + ss->class_word[k][n];
        }
    }
}

void manual_initialize_ss(char *seedfile, blda_suffstats* ss, blda_model* model, blda_corpus* c)
{
    int num_topics = model->num_topics;
    int i, k, d, n, err;
    blda_document* doc;

    FILE *seeds = fopen(seedfile,"r");
    if (seeds == NULL) {
      printf("Couldn't find manual seeds in %s.\n", seedfile);
      exit(1);
    }

    for (k = 0; k < num_topics; k++)
    {
        for (i = 0; i < NUM_INIT; i++)
        {
            err = fscanf(seeds, "%d\n", &d);
            if (err == EOF)
            {
              printf("Ran out of seeds (%d/%d)\n", k, num_topics);
              exit(2);
            } else if (err != 1)
            {
              printf("Couldn't read a seed from ldaseeds.txt. It should have one number per line.\n");
              exit(3);
            }

            doc = &(c->docs[d]);
            for (n = 0; n < doc->length; n++)
            {
                ss->class_word[k][doc->words[n]] += doc->counts[n];
            }
        }
        for (n = 0; n < model->num_terms; n++)
        {
            ss->class_word[k][n] += 1.0;
            ss->class_total[k] = ss->class_total[k] + ss->class_word[k][n];
        }
    }
}


/*
 * allocate new lda model
 *
 */

blda_model* new_lda_model(int num_terms, int num_topics)
{
    int i,j;
    blda_model* model;

    model = (blda_model*)malloc(sizeof(blda_model));
    model->num_topics = num_topics;
    model->num_terms = num_terms;
    model->alpha = 1.0;
    model->log_prob_w = (double**)malloc(sizeof(double*)*num_topics);
    for (i = 0; i < num_topics; i++)
    {
	model->log_prob_w[i] = (double*)malloc(sizeof(double)*num_terms);
	for (j = 0; j < num_terms; j++)
	    model->log_prob_w[i][j] = 0;
    }
    return(model);
}


/*
 * deallocate new lda model
 *
 */

void free_lda_model(blda_model* model)
{
    int i;

    for (i = 0; i < model->num_topics; i++)
    {
	free(model->log_prob_w[i]);
    }
    free(model->log_prob_w);
}


/*
 * save an lda model
 *
 */

void save_lda_model(blda_model* model, char* model_root)
{
    char filename[100];
    FILE* fileptr;
    int i, j;

    sprintf(filename, "%s.beta", model_root);
    fileptr = fopen(filename, "w");
    for (i = 0; i < model->num_topics; i++)
    {
	for (j = 0; j < model->num_terms; j++)
	{
	    fprintf(fileptr, " %5.10f", model->log_prob_w[i][j]);
	}
	fprintf(fileptr, "\n");
    }
    fclose(fileptr);

    sprintf(filename, "%s.other", model_root);
    fileptr = fopen(filename, "w");
    fprintf(fileptr, "num_topics %d\n", model->num_topics);
    fprintf(fileptr, "num_terms %d\n", model->num_terms);
    fprintf(fileptr, "alpha %5.10f\n", model->alpha);
    fclose(fileptr);
}


blda_model* load_lda_model(char* model_root)
{
    char filename[100];
    FILE* fileptr;
    int i, j, num_terms, num_topics;
    float x, alpha;

    sprintf(filename, "%s.other", model_root);
    fileptr = fopen(filename, "r");
    fscanf(fileptr, "num_topics %d\n", &num_topics);
    fscanf(fileptr, "num_terms %d\n", &num_terms);
    fscanf(fileptr, "alpha %f\n", &alpha);
    fclose(fileptr);

    blda_model* model = new_lda_model(num_terms, num_topics);
    model->alpha = alpha;

    sprintf(filename, "%s.beta", model_root);
    fileptr = fopen(filename, "r");
    for (i = 0; i < num_topics; i++)
    {
        for (j = 0; j < num_terms; j++)
        {
            fscanf(fileptr, "%f", &x);
            model->log_prob_w[i][j] = x;
        }
    }
    fclose(fileptr);
    return(model);
}
