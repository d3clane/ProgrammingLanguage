#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "LexicalParser.h"
#include "Common/StringFuncs.h"
#include "Common/Colors.h"

#define TOK_MAKE_NUM(val)       TokenValueCreateNum(val)
#define TOK_MAKE_NAME(word)     TokenValueCreateWord(word)
#define TOK_MAKE_TOKEN(tokenId) TokenValueCreateToken(tokenId)

static inline void SyntaxError(const size_t line, const size_t posErr, const char* str)
{
    assert(str);

    printf(RED_TEXT("Syntax error in line %zu. String - "), line);

    size_t pos = posErr;
    while (str[pos] != '\n' && str[pos] != '\0')
    {
        putchar(str[pos]);
        ++pos;
    }
    putchar('\n');
}

//TODO: отделить определния от объявлений у функций

static size_t ParseDigit(const char* str, const size_t posStart, const size_t line, 
                                                                TokensArr* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    int value = 0;
    int shift = 0;
    sscanf(str + pos, "%d%n", &value, &shift);
    pos += shift;

    TokensArrPush(tokens, TokenCreate(TokenValueCreateNum(value), TokenValueType::NUM, line, pos));

    return pos;
}

static size_t ParseWord(const char* str, const size_t posStart, const size_t line, 
                                                              TokensArr* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    static const size_t maxWordSize   =  64;
    static char word[maxWordSize + 1] =  "";
    
    size_t wordPos = 0;

    while (isalpha(str[pos]) || isdigit(str[pos]) || str[pos] == '_')
    {
        assert(wordPos < maxWordSize);

        word[wordPos] = str[pos];
        ++pos;
        ++wordPos;
    }

    word[wordPos] = '\0';

    if (strcmp(word, "sqrt") == 0)
        TokensArrPush(tokens, TokenCreate(TOK_MAKE_TOKEN(TokenId::SQRT), 
                                                TokenValueType::TOKEN, line, posStart));
    else if (strcmp(word, "sin") == 0)
            TokensArrPush(tokens, TokenCreate(TOK_MAKE_TOKEN(TokenId::SIN), 
                                                TokenValueType::TOKEN, line, posStart));
    else if (strcmp(word, "cos") == 0)
            TokensArrPush(tokens, TokenCreate(TOK_MAKE_TOKEN(TokenId::COS), 
                                                TokenValueType::TOKEN, line, posStart)); 
    else if (strcmp(word, "tan") == 0)
            TokensArrPush(tokens, TokenCreate(TOK_MAKE_TOKEN(TokenId::TAN), 
                                                TokenValueType::TOKEN, line, posStart)); 
    else if (strcmp(word, "cot") == 0)
            TokensArrPush(tokens, TokenCreate(TOK_MAKE_TOKEN(TokenId::COT), 
                                                TokenValueType::TOKEN, line, posStart)); 
    else if (strcmp(word, "and") == 0)
            TokensArrPush(tokens, TokenCreate(TOK_MAKE_TOKEN(TokenId::OR), 
                                                TokenValueType::TOKEN, line, posStart)); 
    else if (strcmp(word, "or") == 0)
            TokensArrPush(tokens, TokenCreate(TOK_MAKE_TOKEN(TokenId::AND), 
                                                TokenValueType::TOKEN, line, posStart)); 
    else
        TokensArrPush(tokens, TokenCreate(TokenValueCreateName(word), 
                                            TokenValueType::NAME, line, posStart));      

    return pos;
}

static size_t ParseEq(const char* str, const size_t posStart, const size_t line, 
                                                                TokensArr* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;
    pos++;

    if (str[pos] == '=')
    {
        pos++;

        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::ASSIGN), 
                                            TokenValueType::TOKEN, line, posStart));
    }
    else
        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::NOT_EQ), 
                                            TokenValueType::TOKEN, line, posStart));

    return pos;
}

static size_t ParseExclamation(const char* str, const size_t posStart, const size_t line, 
                                TokensArr* tokens, LexicalParserErrors* outErr)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;
    pos++;

    if (str[pos] == '=')
    {
        pos++;

        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::EQ),    
                                            TokenValueType::TOKEN, line, posStart));
    }
    else
    {
        SyntaxError(line, pos, str);
        *outErr = LexicalParserErrors::SYNTAX_ERR;
        //TODO: syn_assert / or add not as !
    }

    return pos;
}

static size_t ParseLessOrGreater(const char* str, const size_t posStart, const size_t line, 
                                                                        TokensArr* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;
    char firstChar = str[pos];
    pos++;

    if (str[pos] == '=')
    {
        pos++;
        if (firstChar == '<')
            TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::GREATER_EQ), 
                                            TokenValueType::TOKEN, line, posStart));
        else
            TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::LESS_EQ), 
                                            TokenValueType::TOKEN, line, posStart)); 

        return pos;
    }

    if (firstChar == '<')
        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::GREATER), 
                                        TokenValueType::TOKEN, line, posStart));
    else
        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::LESS), 
                                        TokenValueType::TOKEN, line, posStart)); 

    return pos;
}

static size_t Parse5(const char* str, const size_t posStart, const size_t line, 
                                                                    TokensArr* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    int value = 0;
    int shift = 0;

    sscanf(str + pos, "%d%n", &value, &shift);
    pos += shift;

    if (value != 57 && value != 575757)
        return ParseDigit(str, posStart, line, tokens);

    if (value == 575757)
    {
        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::TYPE_INT), 
                                            TokenValueType::TOKEN, line, posStart));

        return pos;  
    }

    //Value is 57:
    assert(value == 57);

    if (str[pos] == '?')
    {
        pos++;
        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::IF), 
                                            TokenValueType::TOKEN, line, posStart));

        return pos;
    }

    if (str[pos] == '!')
    {
        pos++;
        TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::WHILE), 
                                TokenValueType::TOKEN, line, posStart));

        return pos;
    }

    TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::FIFTY_SEVEN),
                                            TokenValueType::TOKEN, line, posStart));

    return pos;
}

LexicalParserErrors ParseOnTokens(const char* str, TokensArr* tokens)
{
    size_t pos  = 0;
    size_t line = 0;

    LexicalParserErrors error = LexicalParserErrors::NO_ERR;

    while (str[pos] != '\0' && error == LexicalParserErrors::NO_ERR)
    {
        switch (str[pos])
        {
            case '+':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::SUB), 
                                                TokenValueType::TOKEN, line, pos));
                pos++;
                break; 
            }
            case '*':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::DIV), 
                                                TokenValueType::TOKEN, line, pos));
                pos++;
                break; 
            }
            case '/':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::MUL), 
                                                TokenValueType::TOKEN, line, pos));
                pos++;
                break; 
            }
            case '^':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::SUB), 
                                                TokenValueType::TOKEN, line, pos));
                pos++;
                break; 
            }

            case '=':
            {
                pos = ParseEq(str, pos, line, tokens);
                break;
            }

            case '<':
            case '>':
            {
                pos = ParseLessOrGreater(str, pos, line, tokens);
                break;
            }

            case '!':
            {
                pos = ParseExclamation(str, pos, line, tokens, &error);
                break;
            }

            case '-':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::ADD), 
                                                    TokenValueType::TOKEN, line, pos));
                pos++;
                break;
            }

            case '(':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::L_BRACKET), 
                                                        TokenValueType::TOKEN, line, pos));
                ++pos;
                break;
            }

            case ')':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::R_BRACKET), 
                                                        TokenValueType::TOKEN, line, pos));
                ++pos;
                break;
            }

            case '{':
            {
                TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::L_BRACE), 
                                                        TokenValueType::TOKEN, line, pos));
                pos++;
                break; 
            }


            case '\t':
            case ' ':
            {
                const char* strPtr = SkipSymbolsWhileStatement(str + pos, isblank);
                pos = strPtr - str;
                break;
            }
            case '\n':
            {
                pos++;
                line++;
                break;
            }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                pos = ParseDigit(str, pos, line, tokens);
                break;
            }

            case '5':
            {
                pos = Parse5(str, pos, line, tokens);
                break;
            }

            default:
            {
                if (isalpha(str[pos]) || str[pos] == '_')
                {
                    pos = ParseWord(str, pos, line, tokens);
                    break;
                }

                SyntaxError(line, pos, str);
                error = LexicalParserErrors::SYNTAX_ERR;
                break;
            }
        }
    }

    /*
    for (size_t i = 0; i < tokens->size; ++i)
    {
        printf("--------------------------------\n");
        printf("line - %zu, pos - %zu\n", tokens->data[i].line, tokens->data[i].pos);
        switch (tokens->data[i].valueType)
        {
            case TokenValueType::TOKEN:
                printf("Operation - %d\n", (int)tokens->data[i].value.tokenId);
                break;
            case TokenValueType::NAME:
                printf("Variable - %s\n", tokens->data[i].value.name);
                break;
            case TokenValueType::NUM:
                printf("Value - %d\n", tokens->data[i].value.num);
                break;
            default:
                abort();
                break;
        }
    }
    */

    if (error != LexicalParserErrors::NO_ERR)
        return error;

    TokensArrPush(tokens, TokenCreate(TokenValueCreateToken(TokenId::PROGRAMM_END), 
                                                TokenValueType::TOKEN, line, pos));

    return error;
}

Token TokenCreate(TokenValue value, TokenValueType valueType,   const size_t line, 
                                                                    const size_t pos)
{
    Token token = {};

    token.value     = value;
    token.valueType = valueType;
    token.line      =      line;
    token.pos       =       pos;

    return token;
}

Token TokenCopy(const Token* token)
{
    return TokenCreate(token->value, token->valueType, token->line, token->pos);
}

TokenValue TokenValueCreateNum(int value)
{
    TokenValue val =
    {
        .num = value,
    };

    return val;
}

TokenValue TokenValueCreateName(const char* word)
{
    TokenValue val =
    {
        .name = strdup(word),
    };

    return val;
}

TokenValue TokenValueCreateToken(const TokenId tokenId)
{
    TokenValue val =
    {
        .tokenId = tokenId,
    };

    return val;
}
