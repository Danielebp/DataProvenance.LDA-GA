
#include "geneticAlgorithm.h"

void geneticLogic(int population)
{
    int POPULATION_COUNT = 6;
    PopulationConfig[] mInitialPopulation = PopulationConfig.initArray(POPULATION_COUNT);

    boolean maxFitnessFound = false;

    // populating the initial population
    for (int i = 0; i < mInitialPopulation.length; i++) {
        mInitialPopulation[i].random();
    }
    
    int loop_round = 0;
    while (!maxFitnessFound && isRunning) {
        ++loop_round;
        clock_t t = clock();
        
        boolean error = false;
        // send populations to slaves
        NetworkManager.getInstance().waitForAllSlaves();
        mFinishedSlaveCount = 0;
        for (int iSlave = 0; iSlave < NetworkManager.getInstance().getSlaveCount(); ++iSlave) {
            PopulationConfig[] subPopulation = new PopulationConfig[THREADS_PER_MACHINE];
            for (int j = 0; j < THREADS_PER_MACHINE; ++j) {
                subPopulation[j] = mInitialPopulation[THREADS_PER_MACHINE * (iSlave + 1) + j];
            }
            if (!NetworkManager.getInstance().sendProtocol_ProcessSubPopulationNew(iSlave, subPopulation)) {
                error = true;
                break;
            }
        }
        if (error) {
            break;
        }
        /**
         * the total number of documents that are being processed. Put them in a folder
         * and add the folder path here.
         */
        int numberOfDocuments = new File(WikiScrape.ORIGINAL_DATA_DIRECTORY).listFiles().length;

        Thread threads[] = new Thread[THREADS_PER_MACHINE];
        for (int i = 0; i < THREADS_PER_MACHINE; i++) {
            int population_index = THREADS_PER_MACHINE * (NetworkManager.getInstance().getMyMachineID() + 1) + i;
            threads[i] = new Thread(new MyThread(i, mInitialPopulation[population_index], population_index, numberOfDocuments, false));
            // System.out.println("Thread " + i + " begin start...");
            threads[i].start();
            // System.out.println("Thread " + i + " end start...");
        }

        for (int i = 0; i < THREADS_PER_MACHINE; i++) {
            threads[i].join();
            // System.out.println("Thread " + i + " joined");
        }
        NetworkManager.getInstance().dispatchProtocols();

        // receive populations from slaves
        while (mFinishedSlaveCount < NetworkManager.getInstance().getSlaveCount() && isRunning) {
            NetworkManager.getInstance().dispatchProtocols();
            Thread.sleep(1);
        }
        for(int i=0; i<mInitialPopulation.length; ++i)
        {
            result.OnLDAFinish(mInitialPopulation[i]);
        }

        if (!isRunning) {
            break;
        }

        clock_t paraEndTime = clock() - t;
        cout << "round " << loop_round << " parallel part takes " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms   "
                            << NetworkManager.to_string(mInitialPopulation) << endl;

        // ranking and ordering the chromosomes based on the fitness function.
        // no sorting code found?(by Xiaolin)
        // We need only the top 1/3rd of the chromosomes with high fitness values -
        // Silhouette coefficient
        PopulationConfig[] newPopulation = PopulationConfig.initArray(POPULATION_COUNT);
        // copy only the top 1/3rd of the population to the new population
        final int BEST_POPULATION_SIZE = mInitialPopulation.length / 3;
        for (int i = 0; i < BEST_POPULATION_SIZE; i++) {
            double maxFitness = Integer.MIN_VALUE;
            int maxFitnessChromosome = -1;
            for (int j = 0; j < mInitialPopulation.length; j++) {
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
                        TopicModelling tm = new TopicModelling();
                        tm.LDA(mInitialPopulation[j], true, false);
                        System.out.println("The best distribution is: " + mInitialPopulation[j].to_string());
                        result.cfg = mInitialPopulation[j];
                        result.OnLDAFinish(result.cfg);
                        maxFitnessFound = true;
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
            mInitialPopulation[maxFitnessChromosome].fitness_value = Integer.MIN_VALUE;
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
            for (int i = 0; i < newPopulation.length; ++i) {
                newPopulation[i].random();
            }
        } else {
            final double MUTATION_RATIO = 0.5;
            for (int i = BEST_POPULATION_SIZE; i < newPopulation.length; ++i) {
                int iParent = i % BEST_POPULATION_SIZE;
                newPopulation[i].copy(newPopulation[iParent]);
                newPopulation[i].fitness_value = 0;
                if (Math.random() < MUTATION_RATIO) {
                    newPopulation[i].random_topic();
                }
                if (Math.random() < MUTATION_RATIO) {
                    newPopulation[i].random_iteration();
                }
            }
        }

        // substitute the initial population with the new population and continue
        mInitialPopulation = newPopulation;
        SortInitialPopulation();

        t = clock() - t;
        cout << "round " << loop_round << " other part takes " << ((float)t)/(CLOCKS_PER_SEC/1000) << "ms"<<endl;

        /**
         * The genetic algorithm loop will not exit until the required fitness is
         * reached. For some cases, we might expect a very high fitness that will never
         * be reached. In such cases add a variable to check how many times the GA loop
         * is repeated. Terminate the loop in predetermined number of iterations.
         */
    }
    System.out.println("geneticLogic runs " + loop_round + " loops for fitness-threshhold: "+FITNESS_THRESHHOLD+".  "+ NetworkManager.to_string(result.cfg)+"    mInitialPopulation:"+ NetworkManager.to_string(mInitialPopulation));
    return result;   
}