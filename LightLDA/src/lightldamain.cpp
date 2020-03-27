#include "lightlda.h"

int main(int argc, char** argv)
{

       LightLDA mylda;
       mylda.Run(argc, argv);

    	std::cout<<"Main Topic for Doc 0"<<mylda.GetMainTopic(0)<<std::endl;
        std::cout<<"Distributions for Doc 0:"<<std::endl;
        for(int i=0; i<1000; i++)
        std::cout<<i<<": "<<mylda.GetDocTopicDistribution(0, i)<<std::endl;

	return 0;
}


