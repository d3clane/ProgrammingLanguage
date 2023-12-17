#include <assert.h>

#include "BackEnd.h"
#include "Tree/Tree.h"
#include "Common/Log.h"

int main(int argc, char* argv[])
{
    LogOpen(argv[0]);
    setbuf(stdout, nullptr);
    FILE* inStream     = fopen("ParseTree.txt", "r");
    FILE* outStream    = fopen("AsmCode.txt", "w");
    FILE* outBinStream = fopen("code.bin", "w");

    assert(inStream);
    Tree tree = {};
    TreeCtor(&tree);

    NameTableType* allNamesTable = nullptr;
    TreeReadPrefixFormat(&tree, &allNamesTable, inStream);

    AsmCodeBuild(&tree, allNamesTable, outStream, outBinStream);
}