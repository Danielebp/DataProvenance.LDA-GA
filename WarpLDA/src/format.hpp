#ifndef __FORMAT_H__
#define __FORMAT_H__

#define STRIP_FLAG_HELP 0

#include <gflags/gflags.h>

#include "Bigraph.hpp"
#include "Vocab.hpp"
#include "Utils.hpp"
#include <exception>
#include <algorithm>
using namespace std;

template <bool testMode = false>
void parse_document(string &line, std::vector<TVID> &v, Vocab &vocab);

template <bool testMode = false>
void text_to_bin(std::string in, std::string out);

int wlda_format(int argc, char** argv);

#endif
