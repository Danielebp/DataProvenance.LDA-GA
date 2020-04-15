#ifndef __scanner_h__
#define __scanner_h__

#include <string>
#include <iostream>
#include <time.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace std;

class Scanner {

  ifstream myfile;
  stringstream ss;

public:
  Scanner ();
  Scanner (string filename) ;
  void open (string filename) ;
  bool nextLine();
  bool hasNextLine();
  bool nextLineClean();
  void close();

  int nextInt();
  double nextDouble();
  string nextWord();

};

#endif
