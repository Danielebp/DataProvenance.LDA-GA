#include "lightlda.h"

using namespace multiverso::lightlda;     

IDataStream* LightLDA::data_stream = nullptr;
Meta LightLDA::meta;
        
void LightLDA::Run(int argc, char** argv)
        {
            Config::Init(argc, argv);
            
            AliasTable* alias_table = new AliasTable();
            Barrier* barrier = new Barrier(Config::num_local_workers);
            meta.Init();
            std::vector<TrainerBase*> trainers;
            for (int32_t i = 0; i < Config::num_local_workers; ++i)
            {
                Trainer* trainer = new Trainer(alias_table, barrier, &meta);
                trainers.push_back(trainer);
            }

            ParamLoader* param_loader = new ParamLoader();
            multiverso::Config config;
            config.num_servers = Config::num_servers;
            config.num_aggregator = Config::num_aggregator;
            config.server_endpoint_file = Config::server_file;

            Multiverso::Init(trainers, param_loader, config, &argc, &argv);

            Log::ResetLogFile("LightLDA."
                + std::to_string(clock()) + ".log");

            data_stream = CreateDataStream();
            InitMultiverso();
            Train();

            Multiverso::Close();
            
            for (auto& trainer : trainers)
            {
                delete trainer;
            }
            delete param_loader;
            
            DumpDocTopic();

std::cout<<"Main Topic for Doc 0"<<multiverso::lightlda::LightLDA::GetMainTopic(0)<<std::endl;
        std::cout<<"Distributions for Doc 0:"<<std::endl;
        for(int i=0; i<1000; i++)
        std::cout<<i<<": "<<multiverso::lightlda::LightLDA::GetDocTopicDistribution(0, i)<<std::endl;

            delete data_stream;
            delete barrier;
            delete alias_table;
        }
        
void LightLDA::Train()
        {
            Multiverso::BeginTrain();
            for (int32_t i = 0; i < Config::num_iterations; ++i)
            {
                Multiverso::BeginClock();
                // Train corpus block by block
                for (int32_t block = 0; block < Config::num_blocks; ++block)
                {
                    data_stream->BeforeDataAccess();
                    DataBlock& data_block = data_stream->CurrDataBlock();
                    data_block.set_meta(&meta.local_vocab(block));
                    int32_t num_slice = meta.local_vocab(block).num_slice();
                    std::vector<LDADataBlock> data(num_slice);
                    // Train datablock slice by slice
                    for (int32_t slice = 0; slice < num_slice; ++slice)
                    {
                        LDADataBlock* lda_block = &data[slice];
                        lda_block->set_data(&data_block);
                        lda_block->set_iteration(i);
                        lda_block->set_block(block);
                        lda_block->set_slice(slice);
                        Multiverso::PushDataBlock(lda_block);
                    }
                    Multiverso::Wait();
                    data_stream->EndDataAccess();
                }
                Multiverso::EndClock();
            }
            Multiverso::EndTrain();
        }

void LightLDA::InitMultiverso()
        {
            Multiverso::BeginConfig();
            CreateTable();
            ConfigTable();
            Initialize();
            Multiverso::EndConfig();
        }

void LightLDA::Initialize()
        {
            xorshift_rng rng;
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {
                data_stream->BeforeDataAccess();
                DataBlock& data_block = data_stream->CurrDataBlock();
                int32_t num_slice = meta.local_vocab(block).num_slice();
                for (int32_t slice = 0; slice < num_slice; ++slice)
                {
                    for (int32_t i = 0; i < data_block.Size(); ++i)
                    {
                        Document* doc = data_block.GetOneDoc(i);
                        int32_t& cursor = doc->Cursor();
                        if (slice == 0) cursor = 0;
                        int32_t last_word = meta.local_vocab(block).LastWord(slice);
                        for (; cursor < doc->Size(); ++cursor)
                        {
                            if (doc->Word(cursor) > last_word) break;
                            // Init the latent variable
                            if (!Config::warm_start)
                                doc->SetTopic(cursor, rng.rand_k(Config::num_topics));
                            // Init the server table
                            Multiverso::AddToServer<int32_t>(kWordTopicTable,
                                doc->Word(cursor), doc->Topic(cursor), 1);
                            Multiverso::AddToServer<int64_t>(kSummaryRow,
                                0, doc->Topic(cursor), 1);
                        }
                    }
                    Multiverso::Flush();
                }
                data_stream->EndDataAccess();
            }
        }

void LightLDA::DumpDocTopic()
        {
            Row<int32_t> doc_topic_counter(0, Format::Sparse, kMaxDocLength); 
            for (int32_t block = 0; block < Config::num_blocks; ++block)
            {
                std::ofstream fout("doc_topic." + std::to_string(block));
                data_stream->BeforeDataAccess();
                DataBlock& data_block = data_stream->CurrDataBlock();
                for (int i = 0; i < data_block.Size(); ++i)
                {
                    Document* doc = data_block.GetOneDoc(i);
                    doc_topic_counter.Clear();
                    doc->GetDocTopicVector(doc_topic_counter);
                    fout << i << " ";  // doc id
                    Row<int32_t>::iterator iter = doc_topic_counter.Iterator();
                    while (iter.HasNext())
                    {
                        fout << " " << iter.Key() << ":" << iter.Value();
                        iter.Next();
                    }
                    fout << std::endl;
                }
                data_stream->EndDataAccess();
            }
        }

int LightLDA::GetMainTopic(int docID, int block)
        {
	    std::cout<<"Start Get Main"<<std::endl;
            int maxDist = 0;
            int maxID = -1;
            Row<int32_t> doc_topic_counter(0, Format::Sparse, kMaxDocLength);
	    std::cout<<"Created Row"<<std::endl;
            if (block < Config::num_blocks)
            {
                data_stream->BeforeDataAccess();
                std::cout<<"Created Stream"<<std::endl;
                DataBlock& data_block = data_stream->CurrDataBlock();
                std::cout<<"Got current"<<std::endl;
                if (docID < data_block.Size())
                {
                    Document* doc = data_block.GetOneDoc(docID);
                    doc_topic_counter.Clear();
                    doc->GetDocTopicVector(doc_topic_counter);
                    Row<int32_t>::iterator iter = doc_topic_counter.Iterator();
                    while (iter.HasNext())
                    {
                        if(iter.Value() > maxDist){
                            maxID = iter.Key();
                            maxDist = iter.Value();
                        }
                        iter.Next();
                    }
                }
                data_stream->EndDataAccess();
            }
	    return maxID;
        }

double LightLDA::GetDocTopicDistribution(int docID, int topicID, int block)
        {
            int totalWords = 0;
            int topicWords = 0;
            Row<int32_t> doc_topic_counter(0, Format::Sparse, kMaxDocLength);
            if (block < Config::num_blocks)
            {
                data_stream->BeforeDataAccess();
                DataBlock& data_block = data_stream->CurrDataBlock();
                if (docID < data_block.Size())
                {
                    Document* doc = data_block.GetOneDoc(docID);
                    doc_topic_counter.Clear();
                    doc->GetDocTopicVector(doc_topic_counter);
                    Row<int32_t>::iterator iter = doc_topic_counter.Iterator();
                    while (iter.HasNext())
                    {
                        if(iter.Key() == topicID){
                            topicWords = iter.Value();
                        }
                        totalWords += iter.Value();
                        iter.Next();
                    }
                }
                data_stream->EndDataAccess();
            }
            return ((double)topicWords)/totalWords;
        }

void LightLDA::CreateTable()
        {
            int32_t num_vocabs = Config::num_vocabs;
            int32_t num_topics = Config::num_topics;
            Type int_type = Type::Int;
            Type longlong_type = Type::LongLong;
            multiverso::Format dense_format = multiverso::Format::Dense;
            multiverso::Format sparse_format = multiverso::Format::Sparse;

            Multiverso::AddServerTable(kWordTopicTable, num_vocabs,
                num_topics, int_type, dense_format);
            Multiverso::AddCacheTable(kWordTopicTable, num_vocabs,
                num_topics, int_type, dense_format, Config::model_capacity);
            Multiverso::AddAggregatorTable(kWordTopicTable, num_vocabs,
                num_topics, int_type, dense_format, Config::delta_capacity);

            Multiverso::AddTable(kSummaryRow, 1, Config::num_topics,
                longlong_type, dense_format);
        }
        
void LightLDA::ConfigTable()
        {
            multiverso::Format dense_format = multiverso::Format::Dense;
            multiverso::Format sparse_format = multiverso::Format::Sparse;
            for (int32_t word = 0; word < Config::num_vocabs; ++word)
            {
                if (meta.tf(word) > 0)
                {
                    if (meta.tf(word) * kLoadFactor > Config::num_topics)
                    {
                        Multiverso::SetServerRow(kWordTopicTable,
                            word, dense_format, Config::num_topics);
                        Multiverso::SetCacheRow(kWordTopicTable,
                            word, dense_format, Config::num_topics);
                    }
                    else
                    {
                        Multiverso::SetServerRow(kWordTopicTable,
                            word, sparse_format, meta.tf(word) * kLoadFactor);
                        Multiverso::SetCacheRow(kWordTopicTable,
                            word, sparse_format, meta.tf(word) * kLoadFactor);
                    }
                }
                if (meta.local_tf(word) > 0)
                {
                    if (meta.local_tf(word) * 2 * kLoadFactor > Config::num_topics)
                        Multiverso::SetAggregatorRow(kWordTopicTable, 
                            word, dense_format, Config::num_topics);
                    else
                        Multiverso::SetAggregatorRow(kWordTopicTable, word, 
                            sparse_format, meta.local_tf(word) * 2 * kLoadFactor);
                }
            }
        }
