#ifndef LEXICAL_PARSER_H
#define LEXICAL_PARSER_H

#include <stddef.h>
#include "TokensArr/TokensArr.h"

enum class LexicalParserErrors
{
    NO_ERR,

    SYNTAX_ERR,
};

Token TokenCopy(const Token* token);
Token TokenCreate(TokenValue value, TokenValueType valueType,   const size_t line, 
                                                                const size_t pos);

TokenValue TokenValueCreate (const char* name);
TokenValue TokenValueCreate (const int value);
TokenValue TokenValueCreate (const TokenId tokenId);

LexicalParserErrors ParseOnTokens(const char* str, TokensArr* tokens);

#endif 