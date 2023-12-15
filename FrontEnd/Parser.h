#ifndef PARSER_H
#define PARSER_H

#include "Tree/Tree.h"

union TokenValue
{
    TreeOperationId operation;
    char*           word;
    int             val;
};

enum class TokenValueType
{
    KEY_WORD,
    OPERATION,
    VARIABLE,
    VALUE,
};

struct TokenType
{
    TokenValue     value;
    TokenValueType valueType;

    size_t line;
    size_t pos;
};

enum class ParseErrors
{
    NO_ERR,

    SYNTAX_ERR,
};

TokenType TokenCopy(const TokenType* token);
TokenType TokenCreate(TokenValue value, TokenValueType valueType,   const size_t line, 
                                                                    const size_t pos);

TokenValue TokenValueCreateWord(const char* word);
TokenValue TokenValueCreateNum(int value);
TokenValue TokenValueCreateOp(const char* word);

TreeType CodeParse(const char* str);

#endif