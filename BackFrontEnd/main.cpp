#include "BackFrontEnd.h"
#include "Common/Log.h"

int main(int argc, char* argv[])
{
    LogOpen(argv[0]);

    FILE* inStream  = fopen("ParseTree.txt", "r");
    FILE* outStream = fopen("code.txt", "w");

    NameTableType* allNamesTable = nullptr;
    Tree tree = {};
    TreeReadPrefixFormat(&tree, &allNamesTable, inStream);
    
    CodeBuild(&tree, allNamesTable, outStream);
}