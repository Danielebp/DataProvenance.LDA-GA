
#include "utils.h"

string getTimestamp() {
   time_t now = time(0);
   tm *ltm = localtime(&now);

   stringstream ss;
   // print various components of tm structure.
   ss << 1900 + ltm->tm_year << 1 + ltm->tm_mon<< ltm->tm_mday << 1 + ltm->tm_hour << 1 + ltm->tm_min << 1 + ltm->tm_sec;

   return ss.str();
}

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

vector<string> split(string& s, char delimiter)
{
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}
