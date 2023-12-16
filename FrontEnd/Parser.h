#ifndef PARSER_H
#define PARSER_H

#include "Tree/Tree.h"

union TokenValue
{
    TreeOperationId operation;
    char*           name;
    int             num;
};

enum class TokenValueType
{
    ADD,
    SUB,
    MUL,
    DIV,

    POW,
    SQRT,
    SIN,
    COS,
    TAN,
    COT,

    L_BRACKET, 
    R_BRACKET,

    ASSIGN,
    
    IF,
    WHILE,

    PROGRAMM_END, 

    LESS,
    GREATER,
    LESS_EQ,
    GREATER_EQ,
    EQ,
    NOT_EQ,
    
    AND,
    OR,
    
    PRINT,
    READ,
    TYPE_INT,

    FIFTY_SEVEN,

    BLOCK_END,

    NAME,
    NUM,
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

TokenValue TokenValueCreateWord(const char* name);
TokenValue TokenValueCreateNum(int value);
TokenValue TokenValueCreateOp(const char* name);

TreeType CodeParse(const char* str);

#endif