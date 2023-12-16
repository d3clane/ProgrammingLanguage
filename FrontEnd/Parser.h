#ifndef PARSER_H
#define PARSER_H

#include "Tree/Tree.h"

enum class TokenId
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

    L_BRACE,
};

union TokenValue
{
    TokenId      tokenId;
    char*           name;
    int             num;
};

enum class TokenValueType
{
    TOKEN,
    NAME,
    NUM,
};

struct Token
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

Token TokenCopy(const Token* token);
Token TokenCreate(TokenValue value, TokenValueType valueType,   const size_t line, 
                                                                    const size_t pos);

TokenValue TokenValueCreateName(const char* name);
TokenValue TokenValueCreateNum(int value);
TokenValue TokenValueCreateToken(const TokenId tokenId);

TreeType CodeParse(const char* str);

#endif