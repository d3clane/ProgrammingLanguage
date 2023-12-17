#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

enum class AssemblerErrors
{
    NO_ERR,

    SYNTAX_ERR,
};

AssemblerErrors Assemble(const char* inStreamName, const char* outStreamName);
AssemblerErrors Assemble(FILE* inStream, FILE* outStream);

#endif // ASSEMBLER_H