#include "populationConfig.h"

PopulationConfig::PopulationConfig(long _seed) {seed = _seed;}
PopulationConfig::PopulationConfig() {seed = time(NULL);}

void PopulationConfig::set_max(int max_topic, int max_iteration){
    this->MAX_TOPIC_COUNT = max_topic;
    this->MAX_ITERATION_COUNT = max_iteration;
}

void PopulationConfig::copy(PopulationConfig rhs) {
	number_of_topics = rhs.number_of_topics;
	number_of_iterations = rhs.number_of_iterations;
	fitness_value = rhs.fitness_value;
	LDA_execution_milliseconds = rhs.LDA_execution_milliseconds;
	seed = rhs.seed;
}

void PopulationConfig::random()
{
	this->random_topic();
	this->random_iteration();
}

void PopulationConfig::random_topic()
{
	seed = time(NULL);
        this->number_of_topics = (int) floor(getRandomFloat() * (MAX_TOPIC_COUNT-MIN_TOPIC_COUNT+1) + MIN_TOPIC_COUNT);
	this->fitness_value = 0;
}

void PopulationConfig::random_iteration()
{
	seed = time(NULL);
        this->number_of_iterations = (int) floor(getRandomFloat() * (MAX_ITERATION_COUNT-MIN_ITERATION_COUNT+1) + MIN_ITERATION_COUNT);
	this->fitness_value = 0;
}

string PopulationConfig::to_string()
{
	string str = "number_of_topics:" + std::to_string(number_of_topics) + "  number_of_iterations:" + std::to_string(number_of_iterations)
				+ "  fitness_value:" + std::to_string(fitness_value);
	return str;
}

string PopulationConfig::to_string_all()
{
	string str = "number_of_topics:" + std::to_string(number_of_topics) + "  number_of_iterations:" + std::to_string(number_of_iterations)
				+ "  fitness_value:" + std::to_string(fitness_value) + "  LDA_execution_milliseconds:" + std::to_string(LDA_execution_milliseconds);
	return str;
}

PopulationConfig* initArray(int count) {
	PopulationConfig* array = new PopulationConfig[count];
	for (int i = 0; i < count; i++) {
		array[i] = PopulationConfig(time(NULL));
	}
	return array;
}

string to_string(PopulationConfig* population, int count) {
	string result = "";

	for (int i = 0; i < count; i++) {
		result += " " + population[i].to_string();
	}

	return result;
}
