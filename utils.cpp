
#include "utils.h"

float getRandomFloat()
{
    return (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
}

bool isNumber(string s)
{
    bool isNumb = false;
    if(isDigit(s[0]))
    {
        isNumb = true;
        for (int i=1; i < s.length(); i++) 
        {
            if(!isDigit(s[i])) {
                isNumb = false;
                break;
            }
        }
    }

    return isNumb;
}

bool isDigit(char c)
{
    for(char digit = '0'; digit <= '9'; digit++) {
        if (c == digit) return true;
    }

    if (c == '.' || c == ',') return true;

    return false;
}

string standardizeToLower(string s)
{
    int i=0;
    string lower = "";
    while (s[i])
    {
        if((s[i]>='a' && s[i]<='z')||(s[i]>='A' && s[i]<='Z')||(s[i]=='_')) lower += tolower(s[i]);
        i++;
    }
    return lower;
}

string getFileName (const string& filepath)
{
  std::size_t found = filepath.find_last_of("/\\");
  return filepath.substr(found+1);
}

string trim(string str)
{
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
    {
        return "";
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}