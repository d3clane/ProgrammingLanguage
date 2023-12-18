#include <stdio.h>

#include "MiddleEnd.h"
#include "Common/Log.h"

int main(int argc, char* argv[])
{
    LogOpen(argv[0]);
    
    FILE* inStream = fopen("ParseTree.txt", "r");

    Tree tree = {};
    NameTableType* nameTable = nullptr;
    TreeReadPrefixFormat(&tree, &nameTable, inStream);

    TreeSimplify(&tree);

    TreeGraphicDump(&tree, true, nameTable);
}
