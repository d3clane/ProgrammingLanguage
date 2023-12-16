#ifndef SYNTAX_PARSER_H
#define SYNTAX_PARSER_H

#include "Tree/Tree.h"

enum class SyntaxParserErrors
{
    NO_ERR,

    SYNTAX_ERR,
};

TreeType CodeParse(const char* str, SyntaxParserErrors* outErr);

#endif