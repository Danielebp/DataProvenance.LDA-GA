
#include "utils.h"

float getRandomFloat() {
    return (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
}

string getFileName (const string& filepath) {
  std::size_t found = filepath.find_last_of("/\\");
  return filepath.substr(found+1);
}

string trim(string str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
    {
        return "";
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}
