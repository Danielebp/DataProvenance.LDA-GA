#include "resultStatistics.h"

string ResultStatistics::to_string(string head) {
	string time = "Execution Time:" + time_to_str(execution_milliseconds, 30);
	string precision = "Precision: " + std::to_string(precision_percentage);
	string recall = "Recall: " + std::to_string(recall_percentage);
    string gaCalls = "Number of GA calls" + std::to_string(GA_count);
	string ldaCalls = "Number of LDA calls" + std::to_string(LDA_count);
    string ldaTime = "";
	if (LDA_count > 0) {
		ldaTime = "LDA-average-time:" + time_to_str(LDA_time / LDA_count, 0);
	}

	string str = head + "\n" +
                    time + "\n" +
                    precision + "\n" +
                    recall + "\n" +
                    cfg.to_string() + "\n" +
                    gaCalls + "\n" +
                    ldaCalls + "\n" +
                    ldaTime;
	return str;
}

string ResultStatistics::time_to_str(long milliseconds, unsigned long string_min_length) {
	long hours = milliseconds / 1000 / 60 / 60;
	long minutes = (milliseconds / 1000 / 60) % 60;
	long seconds = (milliseconds / 1000) % 60;
	long ms = milliseconds % 1000;
	string str = std::to_string(milliseconds) + "ms (";
	if (hours > 0) {
		str += std::to_string(hours) + "h ";
	}
	if (minutes > 0) {
		str += std::to_string(minutes) + "m ";
	}
	if (seconds > 0) {
		str += std::to_string(seconds) + "s ";
	}
	if (ms > 0) {
		str += std::to_string(ms) + "ms";
	}
	str += ") ";
	while (str.length() < string_min_length) {
		str += " ";
	}
	return str;
}

void ResultStatistics::OnLDAFinish(PopulationConfig cfg) {
	if(cfg.LDA_execution_milliseconds>=0)
	{
		LDA_time += cfg.LDA_execution_milliseconds;
		++LDA_count;
	}
	if(cfg.fitness_value==0.0f)
	{
		cout<<"fitness_value should not be 0 : "<<endl;
	}
}
