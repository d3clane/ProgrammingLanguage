#ifndef BACK_END_H
#define BACK_END_H

#include "Tree/Tree.h"
#include "Tree/NameTable/NameTable.h"

void AsmCodeBuild(Tree* tree, NameTableType* allNamesTable, FILE* outStream, FILE* outBinStream);

#endif