#include <assert.h>

#include "BackFrontEnd.h"
#include "Common/Log.h"

int main(int argc, char* argv[])
{
    assert(argc > 2);
    LogOpen(argv[0]);

    FILE* inStream  = fopen(argv[1], "r");
    FILE* outStream = fopen(argv[2], "w");

    NameTableType* allNamesTable = nullptr;
    Tree tree = {};
    TreeReadPrefixFormat(&tree, &allNamesTable, inStream);
    
    CodeBuild(&tree, allNamesTable, outStream);
}