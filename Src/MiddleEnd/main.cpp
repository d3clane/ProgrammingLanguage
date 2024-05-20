#include <assert.h>

#include <stdio.h>

#include "MiddleEnd.h"
#include "Common/Log.h"

int main(int argc, char* argv[])
{
    assert(argc > 2);
    LogOpen(argv[0]);

    FILE* inStream  = fopen(argv[1], "r");
    FILE* outStream = fopen(argv[2], "w");

    Tree tree = {};
    TreeCtor(&tree);
    
    NameTableType* nameTable = nullptr;
    TreeReadPrefixFormat(&tree, &nameTable, inStream);

    TreeGraphicDump(&tree, true, nameTable);
    
    TreeSimplify(&tree);

    TreeGraphicDump(&tree, true, nameTable);
    TreePrintPrefixFormat(&tree, outStream, nameTable);

    NameTableDtor(nameTable);
    TreeDtor(&tree);
    fclose(inStream);
    fclose(outStream);
}
