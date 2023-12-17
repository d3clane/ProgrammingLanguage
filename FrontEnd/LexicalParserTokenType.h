#ifndef LEXICAL_PARSER_TOKEN_TYPE
#define LEXICAL_PARSER_TOKEN_TYPE

#include <stddef.h>

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

    PROGRAM_END, 

    LESS,
    GREATER,
    LESS_EQ,
    GREATER_EQ,
    EQ,
    NOT_EQ,
    
    AND,
    OR,
    
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

#endif