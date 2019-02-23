#include "resultStatistics.h"

// string ResultStatistics::to_string(string head) {
// 	string time = "Execution_time:" + time_to_str(execution_milliseconds, 30);
// 	string precision = std::to_string(precision_percentage);
// 	string recall = std::to_string(recall_percentage);
// 	string lda = std::to_string(LDA_count);
// 	if (LDA_count > 0) {
// 		lda += "  LDA-average-time:" + time_to_str(LDA_time / LDA_count, 0);
// 	}
//
// 	string str = head + "  " + time + "  " + precision + "  " + recall + "  " + (cfg != NULL ? cfg.to_string() : "") + "  " + lda;
// 	return str;
// }
//
// string ResultStatistics::time_to_str(long milliseconds, long string_min_length) {
// 	long hours = milliseconds / 1000 / 60 / 60;
// 	long minutes = (milliseconds / 1000 / 60) % 60;
// 	long seconds = (milliseconds / 1000) % 60;
// 	long ms = milliseconds % 1000;
// 	string str = std::to_string(milliseconds);
// 	if (hours > 0) {
// 		str += hours + "h ";
// 	}
// 	if (minutes > 0) {
// 		str += minutes + "m ";
// 	}
// 	if (seconds > 0) {
// 		str += seconds + "s ";
// 	}
// 	if (ms > 0) {
// 		str += ms + "ms";
// 	}
// 	str += ") ";
// 	while (str.length() < string_min_length) {
// 		str += " ";
// 	}
// 	return str;
// }

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
