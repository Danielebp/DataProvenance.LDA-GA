#ifndef __LIGHTLDA_H__
#define __LIGHTLDA_H__

#include "common.h"
#include "trainer.h"
#include "alias_table.h"
#include "data_stream.h"
#include "data_block.h"
#include "document.h"
#include "meta.h"
#include "util.h"
#include <vector>
#include <iostream>
#include <multiverso/barrier.h>
#include <multiverso/log.h>
#include <multiverso/row.h>
#include <map>
#include <fstream>


namespace multiverso{ namespace lightlda{

    class LightLDA
    {
    public:
	LightLDA();
	~LightLDA();
        void Run(int argc, char** argv, std::string outDir);
        double GetDocTopicDistribution(int docID, int topicID, int block = 0);
        int GetMainTopic(int docID, int block = 0);
        std::vector<double> GetTopicDistributions(int numDocs, int numTopics, int block = 0);
    private:
        void Train();
        void InitMultiverso();
        void Initialize();
        void DumpDocTopic(std::string outDir);
        void CreateTable();
        void ConfigTable();
    public:
        /*! \brief training data access */
        IDataStream* data_stream;
        /*! \brief training data meta information */
        Meta meta;
    };
    //IDataStream* LightLDA::data_stream = nullptr;
    //Meta LightLDA::meta;

} // namespace lightlda
} // namespace multiverso

#endif

