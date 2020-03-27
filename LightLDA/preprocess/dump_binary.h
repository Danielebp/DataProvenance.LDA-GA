/*!
 * \file dump_binary.cpp
 * \brief Preprocessing tool for converting LibSVM data to LightLDA input binary format
 *  Usage: 
 *    dump_binary <libsvm_input> <word_dict_file_input> <binary_output_dir> <output_file_offset>
 */

#ifndef __DUMP_BINARY_H__
#define __DUMP_BINARY_H__

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <fstream>
#include "../../gldaCuda/src/strtokenizer.h"

#define BUFF_SIZE_LONG  1000000

namespace lightlda
{
    /* 
     * Output file format:
     * 1, the first 4 byte indicates the number of docs in this block
     * 2, the 4 * (doc_num + 1) bytes indicate the offset of reach doc
     * an example
     * 3    // there are 3 docs in this block
     * 0    // the offset of the 1-st doc
     * 10   // the offset of the 2-nd doc, with this we know the length of the 1-st doc is 5 = 10/2
     * 16   // the offset of the 3-rd doc, with this we know the length of the 2-nd doc is 3 = (16-10)/2
     * 24   // with this, we know the length of the 3-rd doc is 4 = (24 - 16)/2
     * w11 t11 w12 t12 w13 t13 w14 t14 w15 t15  // the token-topic list of the 1-st doc
     * w21 t21 w22 t22 w23 t23                     // the token-topic list of the 2-nd doc
     * w31 t31 w32 t32 w33 t33 w34 t34             // the token-topic list of the 3-rd doc

     * the class block_stream helps generate such binary format file, usage:
     * int doc_num = 3;
     * int64_t* offset_buf = new int64_t[doc_num + 1];
     *
     * block_stream bs;
     * bs.open("block");
     * bs.write_empty_header(offset_buf, doc_num);
     * ...
     * // update offset_buf and doc_num...

     * bs.write_doc(doc_buf, doc_idx);
     * ...
     * bs.write_real_header(offset_buf, doc_num);
     * bs.close();
     */
    class block_stream
    {
    public:
        block_stream();
        ~block_stream();
        bool open(const std::string file_name);
        bool write_doc(int32_t* int32_buf, int32_t count);
        bool write_empty_header(int64_t* int64_buf, int64_t count);
        bool write_real_header(int64_t* int64_buf, int64_t count);
        bool seekp(int64_t pos);
        bool close();
    private:
        // assuming each doc has 500 tokens in average, 
        // the block_buf_ will hold 1 million document,
        // needs 0.8GB RAM.
        const int32_t block_buf_size_ = 1024 * 1024 * 2 * 100;

        std::ofstream stream_;
        std::string file_name_;

        int32_t *block_buf_;
        int32_t buf_idx_;

        block_stream(const block_stream& other) = delete;
        block_stream& operator=(const block_stream& other) = delete;
    };

    /*
    (1) open an utf-8 encoded file in binary mode,
    get its content line by line. Working around the CTRL-Z issue in Windows text file reading.
    (2) assuming each line ends with '\n'
    */
    class utf8_stream
    {
    public:
        utf8_stream();
        ~utf8_stream();

        bool open(const std::string& file_name);

        /*
        return true if successfully get a line (may be empty), false if not.
        It is user's task to verify whether a line is empty or not.
        */
        bool getline(std::string &line);
        int64_t count_line();
        bool close();
    private:
        bool block_is_empty();
        bool fill_block();
        std::ifstream stream_;
        std::string file_name_;
        const int32_t block_buf_size_ = 1024 * 1024 * 800;
        // const int32_t block_buf_size_ = 2;
        std::string block_buf_;
        std::string::size_type buf_idx_;
        std::string::size_type buf_end_;

        utf8_stream(const utf8_stream& other) = delete;
        utf8_stream& operator=(const utf8_stream& other) = delete;
    };

}

struct Token {
    int32_t word_id;
    int32_t topic_id;
};

int Compare(const Token& token1, const Token& token2);

double get_time();

void split_string(std::string& line, char separator, std::vector<std::string>& output, bool trimEmpty = false);

void count_doc_num(std::string input_doc, int64_t &doc_num);

void load_global_tf(std::unordered_map<int32_t, int32_t>& global_tf_map,
    std::string word_tf_file,
    int64_t& global_tf_count);

int createLibsvmFile(std::string dfile, std::string libsvmFile, std::string wordmapfile, int ndocs, std::string delimiter) ;

int createBinaryFile(int argc, char* argv[]);


#endif

