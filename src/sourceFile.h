#ifndef __sourceFile_h__
#define __sourceFile_h__

#include <string>
#include "document.h"

using namespace std;

class SourceFile : public Document{

public:
    SourceFile() : Document(){ }
    SourceFile(string name, string keywords) : Document(name, keywords){ }
};

#endif
