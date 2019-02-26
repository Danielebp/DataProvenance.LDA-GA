
#include "geneticAlgorithm.h"

ResultStatistics geneticLogic(int population, int numberOfDocuments)
{
    ResultStatistics result;

    PopulationConfig* mInitialPopulation = initArray(population);

    bool maxFitnessFound = false;

    // populating the initial population
    for (int i = 0; i < population; i++) {
        mInitialPopulation[i].random();
    }

    int loop_round = 0;
    while (!maxFitnessFound) {
        ++loop_round;
        clock_t t = clock();

        bool error = false;

        for (int i = 0; i < population; i++) {
          int population_index = i;
            parallelizableStart(i, mInitialPopulation[population_index], population_index, numberOfDocuments, false);
        }

        for(int i=0; i<population; ++i)
        {
            result.OnLDAFinish(mInitialPopulation[i]);
        }

        clock_t paraEndTime = clock() - t;
        cout << "round " << loop_round << " parallelizable part takes " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms" << endl;

        // ranking and ordering the chromosomes based on the fitness function.
        // no sorting code found?(by Xiaolin)
        // We need only the top 1/3rd of the chromosomes with high fitness values -
        // Silhouette coefficient
        PopulationConfig* newPopulation = initArray(population);
        // copy only the top 1/3rd of the population to the new population
        int BEST_POPULATION_SIZE = population / 3;
        for (int i = 0; i < BEST_POPULATION_SIZE; i++) {
            double maxFitness = DBL_MIN;
            int maxFitnessChromosome = -1;
            for (int j = 0; j < population; j++) {
                if (mInitialPopulation[j].fitness_value > maxFitness) {
                    maxFitness = mInitialPopulation[j].fitness_value;

                    // stop reproducing or creating new generations if the expected fitness is
                    // reached by one of the machines
                    /**
                     * Please find what would be a suitable fitness to classify the set of documents
                     * that you choose
                     */
                    // set fitness threshold here!!!
                    if (maxFitness > FITNESS_THRESHHOLD) {
                        // run the function again to get the words in each topic
                        // the third parameter states that the topics are to be written to a file
                        // create an instance of the topic modelling class

                        // TO DO:
                        // TopicModelling tm = new TopicModelling();
                        // tm.LDA(mInitialPopulation[j], true, false);
                        // cout << "The best distribution is: " << mInitialPopulation[j].to_string() << endl;
                        // result.cfg = mInitialPopulation[j];
                        // result.OnLDAFinish(result.cfg);
                        // maxFitnessFound = true;
                        break;
                    }
                    maxFitnessChromosome = j;
                }
            }

            if (maxFitnessFound) {
                break;
            }

            // copy the chromosome with high fitness to the next generation
            newPopulation[i].copy(mInitialPopulation[maxFitnessChromosome]);
            mInitialPopulation[maxFitnessChromosome].fitness_value = INT_MIN;
        }

        if (maxFitnessFound) {
            break;
        }

        // perform crossover - to fill the rest of the 2/3rd of the initial Population
        // for(int i = 0 ; i < BEST_POPULATION_SIZE ; i++ ) {
        // newPopulation[(i+1)*2][0] = newPopulation[i][0];
        // newPopulation[(i+1)*2][1] = (int) Math.floor(Math.random()*1000 + 1);
        // newPopulation[(i+1)*2+1][0] = (int) Math.floor(Math.random()*12 + 2);
        // newPopulation[(i+1)*2+1][1] = newPopulation[i][1];
        // }

        // perform crossover - to fill the rest of the 2/3rd of the initial Population
        if (BEST_POPULATION_SIZE <= 0) {
            for (int i = 0; i < population; ++i) {
                newPopulation[i].random();
            }
        } else {
            double MUTATION_RATIO = 0.5;
            for (int i = BEST_POPULATION_SIZE; i < population; ++i) {
                int iParent = i % BEST_POPULATION_SIZE;
                newPopulation[i].copy(newPopulation[iParent]);
                newPopulation[i].fitness_value = 0;
                if (getRandomFloat() < MUTATION_RATIO) {
                    newPopulation[i].random_topic();
                }
                if (getRandomFloat() < MUTATION_RATIO) {
                    newPopulation[i].random_iteration();
                }
            }
        }

        // substitute the initial population with the new population and continue
        mInitialPopulation = newPopulation;

        // TO DO:
        //SortInitialPopulation();

        t = clock() - t;
        cout << "round " << loop_round << " other part takes " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms"<<endl;

        /**
         * The genetic algorithm loop will not exit until the required fitness is
         * reached. For some cases, we might expect a very high fitness that will never
         * be reached. In such cases add a variable to check how many times the GA loop
         * is repeated. Terminate the loop in predetermined number of iterations.
         */
    }
    // TO DO:
    // cout << "geneticLogic runs " << loop_round << " loops for fitness-threshhold: " << FITNESS_THRESHHOLD << ".  "
    // << NetworkManager.to_string(result.cfg) << "    mInitialPopulation:" << NetworkManager.to_string(mInitialPopulation) << endl;
    return result;
}
