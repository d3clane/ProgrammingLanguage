#ifndef LEXICAL_PARSER_H
#define LEXICAL_PARSER_H

#include <stddef.h>
#include "TokensArr/TokensArr.h"

typedef TokensArrType TokensArr;

enum class LexicalParserErrors
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

LexicalParserErrors ParseOnTokens(const char* str, TokensArr* tokens);

#endif 