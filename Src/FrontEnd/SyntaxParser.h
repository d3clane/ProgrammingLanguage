#ifndef SYNTAX_PARSER_H
#define SYNTAX_PARSER_H

#include "Tree/Tree.h"

enum class SyntaxParserErrors
{
    NO_ERR,

    SYNTAX_ERR,
};

void CodeParse(const char* code, SyntaxParserErrors* outErr, FILE* outStream);

#endif