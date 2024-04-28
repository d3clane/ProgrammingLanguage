#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "LexicalParser.h"
#include "Common/StringFuncs.h"
#include "Common/Colors.h"
#include "LexicalParserTokenType.h"

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

#define PUSH_LANG_OP_TOKEN(LANG_OP_ID)                                                      \
    TokensArrPush(tokens, TokenCreate(TokenValueCreate(LANG_OP_ID),                         \
                                                TokenValueType::LANG_OP, line, posStart))

#define PUSH_NAME_TOKEN(WORD)                                                               \
    TokensArrPush(tokens, TokenCreate(TokenValueCreate(WORD),                               \
                                    TokenValueType::NAME, line, posStart));                 \

#define PUSH_NUM_TOKEN(VALUE)                                                               \
    TokensArrPush(tokens, TokenCreate(TokenValueCreate(VALUE), TokenValueType::NUM, line, pos));

static size_t ParseNumber(const char* str, const size_t posStart, const size_t line, 
                                                                TokensArr* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    int value = 0;
    int shift = 0;
    sscanf(str + pos, "%d%n", &value, &shift);
    pos += shift;

    PUSH_NUM_TOKEN(value);

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
        PUSH_LANG_OP_TOKEN(LangOpId::SQRT);
    else if (strcmp(word, "sin") == 0)
        PUSH_LANG_OP_TOKEN(LangOpId::SIN);
    else if (strcmp(word, "cos") == 0)
        PUSH_LANG_OP_TOKEN(LangOpId::COS);
    else if (strcmp(word, "tan") == 0)
            PUSH_LANG_OP_TOKEN(LangOpId::TAN);
    else if (strcmp(word, "cot") == 0)
            PUSH_LANG_OP_TOKEN(LangOpId::COT); 
    else if (strcmp(word, "and") == 0)
            PUSH_LANG_OP_TOKEN(LangOpId::AND);
    else if (strcmp(word, "or") == 0)
            PUSH_LANG_OP_TOKEN(LangOpId::OR);
    else
        PUSH_NAME_TOKEN(word);

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

        PUSH_LANG_OP_TOKEN(LangOpId::ASSIGN);
    }
    else
        PUSH_LANG_OP_TOKEN(LangOpId::NOT_EQ);

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

        PUSH_LANG_OP_TOKEN(LangOpId::EQ);
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
            PUSH_LANG_OP_TOKEN(LangOpId::GREATER_EQ);
        else
            PUSH_LANG_OP_TOKEN(LangOpId::LESS_EQ);

        return pos;
    }

    if (firstChar == '<')
        PUSH_LANG_OP_TOKEN(LangOpId::GREATER);
    else
        PUSH_LANG_OP_TOKEN(LangOpId::LESS); 

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
        return ParseNumber(str, posStart, line, tokens);

    if (value == 575757)
    {
        PUSH_LANG_OP_TOKEN(LangOpId::TYPE_INT);

        return pos;  
    }

    //Value is 57:
    assert(value == 57);

    if (str[pos] == '?')
    {
        pos++;
        PUSH_LANG_OP_TOKEN(LangOpId::IF);

        return pos;
    }

    if (str[pos] == '!')
    {
        pos++;
        PUSH_LANG_OP_TOKEN(LangOpId::WHILE);

        return pos;
    }

    PUSH_LANG_OP_TOKEN(LangOpId::FIFTY_SEVEN);

    return pos;
}

static size_t ParseQuotes(const char* str, const size_t posStart, const size_t line,
                                                                        TokensArr* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    static const size_t maxWordLength  = 1024;
    static char    word[maxWordLength] =   "";

    size_t wordPos = 0;

    do
    {
        word[wordPos] = str[pos];
        wordPos++;
        pos++;

        assert(wordPos < maxWordLength);
    } while (str[pos] != '"');

    word[wordPos] = str[pos];
    pos++;
    wordPos++;
    word[wordPos] = '\0';

    PUSH_NAME_TOKEN(word);

    return pos;                                                
}

#undef  PUSH_LANG_OP_TOKEN
#define PUSH_LANG_OP_TOKEN(LANG_OP_ID)                                                      \
    TokensArrPush(tokens, TokenCreate(TokenValueCreate(LANG_OP_ID),                         \
                                                TokenValueType::LANG_OP, line, pos))        

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
                PUSH_LANG_OP_TOKEN(LangOpId::SUB);
                pos++;
                break; 
            }
            case '*':
            {
                PUSH_LANG_OP_TOKEN(LangOpId::DIV);
                pos++;
                break; 
            }
            case '/':
            {
                PUSH_LANG_OP_TOKEN(LangOpId::MUL);
                pos++;
                break; 
            }
            case '^':
            {
                PUSH_LANG_OP_TOKEN(LangOpId::POW);
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
                PUSH_LANG_OP_TOKEN(LangOpId::ADD);
                pos++;
                break;
            }

            case '(':
            {
                PUSH_LANG_OP_TOKEN(LangOpId::L_BRACKET);
                ++pos;
                break;
            }

            case ')':
            {
                PUSH_LANG_OP_TOKEN(LangOpId::R_BRACKET);
                ++pos;
                break;
            }

            case '{':
            {
                PUSH_LANG_OP_TOKEN(LangOpId::L_BRACE);
                pos++;
                break; 
            }

            case '.':
            {
                PUSH_LANG_OP_TOKEN(LangOpId::PRINT);
                pos++;
                break;
            }

            case '"':
            {
                pos = ParseQuotes(str, pos, line, tokens);
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

            case '@':
            {
                const char* strPtr = SkipSymbolsUntilStopChar(str + pos, '\n');
                pos = strPtr - str;
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
                pos = ParseNumber(str, pos, line, tokens);
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

    assert(error == LexicalParserErrors::NO_ERR);

    /*
    for (size_t i = 0; i < tokens->size; ++i)
    {
        printf("--------------------------------\n");
        printf("line - %zu, pos - %zu, token - %zu\n", tokens->data[i].line, tokens->data[i].pos, i);
        switch (tokens->data[i].valueType)
        {
            case TokenValueType::LANG_OP:
                printf("Operation - %d\n", (int)tokens->data[i].value.langOpId);
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

    PUSH_LANG_OP_TOKEN(LangOpId::PROGRAM_END);

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

TokenValue TokenValueCreate(int value)
{
    TokenValue val =
    {
        .num = value,
    };

    return val;
}

TokenValue TokenValueCreate(const char* word)
{
    TokenValue val =
    {
        .name = strdup(word),
    };

    return val;
}

TokenValue TokenValueCreate(const LangOpId langOpId)
{
    TokenValue val =
    {
        .langOpId = langOpId,
    };

    return val;
}

#undef PUSH_LANG_OP_TOKEN
#undef PUSH_NUM_TOKEN
#undef PUSH_NAME_TOKEN