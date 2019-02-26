#ifndef __scanner_h__
#define __scanner_h__

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class Scanner {

  stringstream ss;
  int found;
  double dfound;

public:

  Scanner (string str) ;

  int nextInt();
  double nextDouble();

};

#endif
