/*!
 * \file dump_binary.cpp
 * \brief Preprocessing tool for converting LibSVM data to LightLDA input binary format
 *  Usage: 
 *    dump_binary <libsvm_input> <word_dict_file_input> <binary_output_dir> <output_file_offset>
 */

#include "dump_binary.h"


using namespace lightlda;

    block_stream::block_stream()
        : buf_idx_(0)
    {
        block_buf_ = new int32_t[block_buf_size_];
    }
    block_stream::~block_stream()
    {
        if (block_buf_)
        {
            delete[]block_buf_;
        }
    }

    bool block_stream::open(const std::string file_name)
    {
        file_name_ = file_name;
        stream_.open(file_name_, std::ios::out | std::ios::binary);
        return stream_.good();
    }

    bool block_stream::seekp(int64_t pos)
    {
        stream_.seekp(pos);
        return true;
    }

    bool block_stream::write_empty_header(int64_t* int64_buf, int64_t count)
    {
        stream_.write(reinterpret_cast<char*>(&count), sizeof(int64_t));
        stream_.write(reinterpret_cast<char*>(int64_buf), 
            sizeof(int64_t)* (count + 1));
        return true;
    }

    bool block_stream::write_real_header(int64_t* int64_buf, int64_t count)
    {
        // clear off the block_buf_, if any content not dumped to disk
        if (buf_idx_ != 0)
        {
            stream_.write(reinterpret_cast<char*> (block_buf_), 
                sizeof(int32_t)* buf_idx_);
            buf_idx_ = 0;
        }

        seekp(0);
        write_empty_header(int64_buf, count);
        return true;
    }

    bool block_stream::write_doc(int32_t* int32_buf, int32_t count)
    {
        if (buf_idx_ + count > block_buf_size_)
        {
            stream_.write(reinterpret_cast<char*>(block_buf_), 
                sizeof(int32_t)* buf_idx_);
            buf_idx_ = 0;
        }
        memcpy(block_buf_ + buf_idx_, int32_buf, count * sizeof(int32_t));
        buf_idx_ += count;
        return true;
    }

    bool block_stream::close()
    {
        stream_.close();
        return true;
    }

    utf8_stream::utf8_stream()
    {
        block_buf_.resize(block_buf_size_);
    }
    utf8_stream::~utf8_stream()
    {
    }

    bool utf8_stream::open(const std::string& file_name)
    {
        stream_.open(file_name, std::ios::in | std::ios::binary);
        buf_idx_ = 0;
        buf_end_ = 0;
        return stream_.good();
    }

    bool utf8_stream::getline(std::string& line)
    {
        line = "";
        while (true)
        {
            if (block_is_empty())
            {
                // if the block_buf_ is empty, fill the block_buf_
                if (!fill_block())
                {
                    // if fail to fill the block_buf_, that means we reach the end of file
                    if (!line.empty())
                        std::cout << "Invalid format, according to our assumption: "
                       "each line has an \\n. However, we reach here with an non-empty line but not find an \\n";
                    return false;
                }
            }
            // the block is not empty now

            std::string::size_type end_pos = block_buf_.find("\n", buf_idx_);
            if (end_pos != std::string::npos)
            {
                // successfully find a new line
                line += block_buf_.substr(buf_idx_, end_pos - buf_idx_);
                buf_idx_ = end_pos + 1;
                return true;
            }
            else
            {
                // do not find an \n untile the end of block_buf_
                line += block_buf_.substr(buf_idx_, buf_end_ - buf_idx_);
                buf_idx_ = buf_end_;
            }
        }
        return false;
    }

    int64_t utf8_stream::count_line()
    {
        char* buffer = &block_buf_[0];

        int64_t line_num = 0;
        while (true)
        {
            stream_.read(buffer, block_buf_size_);
            int32_t end_pos = static_cast<int32_t>(stream_.gcount());
            if (end_pos == 0)
            {
                break;
            }
            line_num += std::count(buffer, buffer + end_pos, '\n');
        }
        return line_num;
    }

    bool utf8_stream::block_is_empty()
    {
        return buf_idx_ == buf_end_;
    }

    bool utf8_stream::fill_block()
    {
        char* buffer = &block_buf_[0];
        stream_.read(buffer, block_buf_size_);
        buf_idx_ = 0;
        buf_end_ = static_cast<std::string::size_type>(stream_.gcount());
        return buf_end_ != 0;
    }

    bool utf8_stream::close()
    {
        stream_.close();
        return true;
    }


int Compare(const Token& token1, const Token& token2) {
    return token1.word_id < token2.word_id;
}

double get_time()
{
    auto start = std::chrono::high_resolution_clock::now();
    auto since_epoch = start.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(since_epoch).count();
}

void split_string(std::string& line, char separator, std::vector<std::string>& output, bool trimEmpty)
{
    output.clear();

    if (line.empty())
    {
        return;
    }

    // trip whitespace, \r
    while (!line.empty())
    {
        int32_t last = line.length() - 1;
        if (line[last] == ' ' || line[last] == '\r')
        {
            line.erase(last, 1);
        }
        else
        {
            break;
        }
    }

    std::string::size_type pos;
    std::string::size_type lastPos = 0;

    using value_type = std::vector<std::string>::value_type;
    using size_type = std::vector<std::string>::size_type;

    while (true)
    {
        pos = line.find_first_of(separator, lastPos);
        if (pos == std::string::npos)
        {
            pos = line.length();

            if (pos != lastPos || !trimEmpty)
                output.push_back(value_type(line.data() + lastPos,
                (size_type)pos - lastPos));

            break;
        }
        else
        {
            if (pos != lastPos || !trimEmpty)
                output.push_back(value_type(line.data() + lastPos,
                (size_type)pos - lastPos));
        }

        lastPos = pos + 1;
    }
    return;
}

void count_doc_num(std::string input_doc, int64_t &doc_num)
{
    lightlda::utf8_stream stream;
    if (!stream.open(input_doc))
    {
        std::cout << "Fails to open file: " << input_doc << std::endl;
        exit(1);
    }
    doc_num = stream.count_line();
    stream.close();
}

void load_global_tf(std::unordered_map<int32_t, int32_t>& global_tf_map,
    std::string word_tf_file,
    int64_t& global_tf_count)
{
    lightlda::utf8_stream stream;
    if (!stream.open(word_tf_file))
    {
        std::cout << "Fails to open file: " << word_tf_file << std::endl;
        exit(1);
    }
    std::string line;
    while (stream.getline(line))
    {
        std::vector<std::string> output;
        split_string(line, '\t', output);
        if (output.size() != 3)
        {
            std::cout << "Invalid line: " << line << std::endl;
            exit(1);
        }
        int32_t word_id = std::stoi(output[0]);
        int32_t tf = std::stoi(output[2]);
        auto it = global_tf_map.find(word_id);
        if (it != global_tf_map.end())
        {
            std::cout << "Duplicate words detected: " << line << std::endl;
            exit(1);
        }
        global_tf_map.insert(std::make_pair(word_id, tf));
        global_tf_count += tf;
    }
    stream.close();
}

int createLibsvmFile(std::string dfile, std::string libsvmFile, std::string wordmapfile, int ndocs, std::string delimiter) {
            std::map<std::string, int> word2id;
            std::map<int, std::string> id2doc;
            std::map<int, int>* wordCountDoc;
            std::map<int, int>wordCountTot;

            FILE * fin = fopen(dfile.c_str(), "r");
            if (!fin) {
            //cfg->logger.log(error, "Cannot open file "+dfile+" to read!");
            return 1;
            }

            std::map<std::string, int>::iterator wordmapIt;
            char buff[BUFF_SIZE_LONG];
            std::string line;

            std::ofstream outDocMap;
            outDocMap.open (libsvmFile);


            for (int docID = 0; docID < ndocs; docID++) {
                fgets(buff, BUFF_SIZE_LONG - 1, fin);

                line = buff;
                int pos = line.find(delimiter);
                std::string fname = line.substr(0, pos);
                id2doc.insert(std::pair<int, std::string>(docID, fname));
                if(pos!=std::string::npos)
                    line.erase(0, pos + 17);

                strtokenizer strtok(line, " \t\r\n");
                int length = strtok.count_tokens();

                if (length <= 0) {
                    //cfg->logger.log(error, "Invalid (empty) document: " + fname);
                    ndocs--;
                    docID--;
                    continue;
                }
                wordCountDoc = new std::map<int, int>();

                for (int token = 0; token < length; token++) {
                    wordmapIt = word2id.find(strtok.token(token));
                    if (wordmapIt == word2id.end()) {
                    // word not found, i.e., new word
                    wordCountDoc->insert(std::pair<int, int>(word2id.size(), 1));
                    word2id.insert(std::pair<std::string, int>(strtok.token(token), word2id.size()));
                    } else {
                        (*wordCountDoc)[wordmapIt->second] = (*wordCountDoc)[wordmapIt->second]+1;
                    }
                }

                // write libsvm file
                outDocMap<<docID<<"\t";
                for (std::pair<int, int> element : (*wordCountDoc)) {
                    outDocMap<<element.first<<":"<<element.second<<" ";

                    // update total count
                    if (wordCountTot.find(element.first) == wordCountTot.end()) {
                        // word not found, i.e., new word
                        wordCountTot.insert(std::pair<int, int>(element.first, element.second));
                    } else {
                        wordCountTot[element.first] = wordCountTot[element.first]+element.second;
                    }
                }
                outDocMap<<std::endl;

                delete wordCountDoc;
            }

            outDocMap.close();
            fclose(fin);

            std::ofstream outWordMap;
            outWordMap.open (wordmapfile);
            for (std::pair<std::string, int> element : word2id) {
                outWordMap<<element.second<<"\t"<<element.first<<"\t"<<wordCountTot[element.second]<<std::endl;
            }
            outWordMap.close();

            return 0;
}

int createBinaryFile(int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("Usage: dump_binary <libsvm_input> <word_dict_file_input> <binary_output_dir> <output_file_offset>\n");
        exit(1);
    }

    std::string libsvm_file_name(argv[1]);
    std::string word_dict_file_name(argv[2]);
    std::string output_dir(argv[3]);
    int32_t output_offset = atoi(argv[4]);
    const int32_t kMaxDocLength = 8192;

    // 1. count how many documents in the data set
    int64_t doc_num;
    count_doc_num(libsvm_file_name, doc_num);

    // 2. load the word_dict file, get the global {word_id, tf} mapping
    std::unordered_map<int32_t, int32_t> global_tf_map;
    std::unordered_map<int32_t, int32_t> local_tf_map;
    int64_t global_tf_count = 0;
    load_global_tf(global_tf_map, word_dict_file_name, global_tf_count);
    int32_t word_num = global_tf_map.size();
    std::cout << "There are totally " << word_num 
			<< " words in the vocabulary" << std::endl;
    std::cout << "There are maximally totally " << global_tf_count 
			<< " tokens in the data set" << std::endl;

    // 3. transform the libsvm -> binary block
    int64_t* offset_buf = new int64_t[doc_num + 1];
    int32_t *doc_buf = new int32_t[kMaxDocLength * 2 + 1];

    std::string block_name = output_dir + "/block." + std::to_string(output_offset);
    std::string vocab_name = output_dir + "/vocab." + std::to_string(output_offset);
    std::string txt_vocab_name = output_dir + "/vocab." + std::to_string(output_offset) + ".txt";

    // open file
    lightlda::utf8_stream libsvm_file;
    lightlda::block_stream block_file;

    if (!libsvm_file.open(libsvm_file_name))
    {
        std::cout << "Fails to open file: " << libsvm_file_name << std::endl;
        exit(1);
    }
    if (!block_file.open(block_name))
    {
        std::cout << "Fails to create file: " << block_name << std::endl;
        exit(1);
    }
    std::ofstream vocab_file(vocab_name, std::ios::out | std::ios::binary);
    std::ofstream txt_vocab_file(txt_vocab_name, std::ios::out);

    if (!vocab_file.good())
    {
        std::cout << "Fails to create file: " << vocab_name << std::endl;
        exit(1);
    }
    if (!txt_vocab_file.good())
    {
        std::cout << "Fails to create file: " << txt_vocab_name << std::endl;
        exit(1);
    }

    block_file.write_empty_header(offset_buf, doc_num);

    int64_t block_token_num = 0;
    std::string str_line;
    std::string line;
    char *endptr = nullptr;
    const int kBASE = 10;
    int doc_buf_idx;

    double dump_start = get_time();

    offset_buf[0] = 0;
    for (int64_t j = 0; j < doc_num; ++j)
    {
        if (!libsvm_file.getline(str_line) || str_line.empty())
        {
            std::cout << "Fails to get line" << std::endl;
            exit(1);
        }
        str_line += '\n';

        std::vector<std::string> output;
        split_string(str_line, '\t', output);


        if (output.size() != 2)
        {
            std::cout << "Invalid format, not key TAB val: " << str_line << std::endl;
            exit(1);
        }

        int doc_token_count = 0;
        std::vector<Token> doc_tokens;

        char *ptr = &(output[1][0]);

        while (*ptr != '\n')
        {
            if (doc_token_count >= kMaxDocLength) break;
            // read a word_id:count pair
            int32_t word_id = strtol(ptr, &endptr, kBASE);
            ptr = endptr;
            if (':' != *ptr)
            {
                std::cout << "Invalid input" << str_line << std::endl;
                exit(1);
            }
            int32_t count = strtol(++ptr, &endptr, kBASE);

            ptr = endptr;
            for (int k = 0; k < count; ++k)
            {
                doc_tokens.push_back({ word_id, 0 });
                if (local_tf_map.find(word_id) == local_tf_map.end())
                {
                    local_tf_map.insert(std::make_pair(word_id, 1));
                }
                else
                {
                    local_tf_map[word_id]++;
                }
                ++block_token_num;
                ++doc_token_count;
                if (doc_token_count >= kMaxDocLength) break;
            }
            while (*ptr == ' ' || *ptr == '\r') ++ptr;
        }
        // The input data may be already sorted
        std::sort(doc_tokens.begin(), doc_tokens.end(), Compare);

        doc_buf_idx = 0;
        doc_buf[doc_buf_idx++] = 0; // cursor

        for (auto& token : doc_tokens)
        {
            doc_buf[doc_buf_idx++] = token.word_id;
            doc_buf[doc_buf_idx++] = token.topic_id;
        }

        block_file.write_doc(doc_buf, doc_buf_idx);
        offset_buf[j + 1] = offset_buf[j] + doc_buf_idx;
    }
    block_file.write_real_header(offset_buf, doc_num);

    int32_t vocab_size = 0;

    vocab_file.write(reinterpret_cast<char*>(&vocab_size), sizeof(int32_t));

    int32_t non_zero_count = 0;
    // write vocab
    for (int i = 0; i < word_num; ++i)
    {
        if (local_tf_map[i] > 0)
        {
            non_zero_count++;
            vocab_file.write(reinterpret_cast<char*> (&i), sizeof(int32_t));
        }
    }
    std::cout << "The number of tokens in the output block is: " << block_token_num << std::endl;
    std::cout << "Local vocab_size for the output block is: " << non_zero_count << std::endl;

    // write global tf
    for (int i = 0; i < word_num; ++i)
    {
        if (local_tf_map[i] > 0)
        {
            vocab_file.write(reinterpret_cast<char*> (&global_tf_map[i]), sizeof(int32_t));
        }
    }
    // write local tf
    for (int i = 0; i < word_num; ++i)
    {
        if (local_tf_map[i] > 0)
        {
            vocab_file.write(reinterpret_cast<char*> (&local_tf_map[i]), sizeof(int32_t));
        }
    }
    vocab_file.seekp(0);
    vocab_file.write(reinterpret_cast<char*>(&non_zero_count), sizeof(int32_t));
    vocab_file.close();

    txt_vocab_file << non_zero_count << std::endl;
    for (int i = 0; i < word_num; ++i)
    {
        if (local_tf_map[i] > 0)
        {
            txt_vocab_file << i << "\t" << global_tf_map[i] << "\t" << local_tf_map[i] << std::endl;
        }
    }
    txt_vocab_file.close();

    double dump_end = get_time();
    std::cout << "Elapsed seconds for dump blocks: " << (dump_end - dump_start) << std::endl;

    // close file and release resource
    libsvm_file.close();
    block_file.close();

    delete[]offset_buf;
    delete[]doc_buf;
    return 0;
}
