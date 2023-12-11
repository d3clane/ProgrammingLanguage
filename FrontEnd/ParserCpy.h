#ifndef PARSER_H
#define PARSER_H

// while = if
// if = while (поменять их местами)
// закрывающая скобка всегда break
// 57!57!57!()
// if ....
// {
// ....
// ....
// ....
// ....
// ? .... ..... 
// 57
// .... 57
// .... 57
// .... 57
// .... 57
// .... 57
// .... 57
// break


#include <stdio.h>
#include "Tree/Tree.h"

struct TokenType
{
    TreeNodeType* node;

    size_t line;
    size_t pos;
};

enum class ParseErrors
{
    NO_ERR,
     
};

TokenType TokenCopy(const TokenType* token);
TokenType TokenCreate(TreeNodeValue value, TreeNodeValueTypeof valueType, const size_t line, 
                                                                          const size_t pos);

TreeType CodeParse(const char* str);


#endif //PARSER_H
