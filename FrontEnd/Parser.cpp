#include <ctype.h>
#include <assert.h>
#include <string.h>

#include "Tree/DSL.h"
#include "Tree/Tree.h"
#include "Parser.h"
#include "Common/StringFuncs.h"
#include "Vector/Vector.h"
#include "Common/Colors.h"

typedef VectorType TokensArrType;

static TreeErrors ParseOnTokens(const char* str, TokensArrType* tokens);

struct DescentStorage
{
    TokensArrType tokens;

    size_t tokenPos;
};

static void DescentStorageCtor(DescentStorage* storage);

static void DescentStorageDtor(DescentStorage* storage);

#define T_CRT_NUM(val)  TokenValueCreateNum(val)
#define T_CRT_VAR(val)  TokenValueCreateWord(word)
#define T_CRT_OP(val)   TokenValueCreateOp(word)

#define  T_OP_TYPE_CNST TokenValueType::OPERATION
#define T_NUM_TYPE_CNST TokenValueType::VALUE
#define T_VAR_TYPE_CNST TokenValueType::VARIABLE

#define POS(storage) storage->tokenPos

static inline const char* T_WORD(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.word;
}

static inline const char* T_WORD(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].value.word;
}

static inline double T_NUM(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.val;
}

static inline double T_NUM(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].value.val;
}

static inline bool T_IS_OP(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].valueType == T_OP_TYPE_CNST;
}

static inline bool T_IS_OP(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == T_OP_TYPE_CNST;
}

static inline bool T_IS_NUM(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].valueType == T_NUM_TYPE_CNST;
}

static inline bool T_IS_NUM(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == T_NUM_TYPE_CNST;
}

static inline bool T_IS_VAR(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].valueType == T_VAR_TYPE_CNST;
}

static inline bool T_IS_VAR(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == T_VAR_TYPE_CNST;
}

static inline bool T_CMP_WORD(const DescentStorage* storage, const char* str)
{
    return (strcmp(T_WORD(storage), str) == 0);
}

static inline bool T_CMP_WORD(const DescentStorage* storage, const size_t pos, const char* str)
{
    return (strcmp(T_WORD(storage, pos), str) == 0);
}

static inline bool T_CMP_WORD(const TokensArrType* tokens, const size_t pos, const char* str)
{
    return (strcmp(tokens->data[pos].value.word, str) == 0);
}

static size_t ParseDigit(const char* str, const size_t posStart, const size_t line, 
                                                                TokensArrType* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    int value = 0;
    int shift = 0;
    sscanf(str + pos, "%d%n", &value, &shift);
    pos += shift;

    VectorPush(tokens, TokenCreate(TokenValueCreateNum(value), TokenValueType::VALUE, line, pos));

    return pos;
}

static size_t ParseWord(const char* str, const size_t posStart, const size_t line, 
                                                              TokensArrType* tokens)
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

    if (strcmp(word, "sqrt") == 0 || strcmp(word, "pow") == 0 || strcmp(word, "sin") == 0 ||
        strcmp(word, "cos")  == 0 || strcmp(word, "tan") == 0 || strcmp(word, "cot") == 0 ||
        strcmp(word, "or")   == 0 || strcmp(word, "and") == 0)
    {
        VectorPush(tokens, TokenCreate(T_CRT_OP(word), TokenValueType::OPERATION, line, posStart));

        return pos;
    }

    VectorPush(tokens, TokenCreate(TokenValueCreateWord(word), 
                                        TokenValueType::VARIABLE, line, posStart));

    return pos;
}

static size_t ParseChar(const char* str, const size_t posStart, const size_t line, 
                                                                TokensArrType* tokens)
{
    assert(str);
    assert(tokens);
    
    static const size_t  charWordLength  =  2;
    static char charWord[charWordLength] = "";

    charWord[0] = str[posStart];
    charWord[1] = '\0';

    VectorPush(tokens, TokenCreate(TokenValueCreateOp(charWord), 
                                        TokenValueType::OPERATION, line, posStart));

    return posStart + 1;
}

static size_t ParseEq(const char* str, const size_t posStart, const size_t line, 
                                                                TokensArrType* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    static const size_t  wordLength  =  3;
    static char     word[wordLength] = "";

    word[0] = str[pos];
    pos++;

    word[1] = '\0';

    if (str[pos] == '=')
    {
        word[1] = '=';
        pos++;
        word[2] = '\0';
    }

    VectorPush(tokens, TokenCreate(TokenValueCreateOp(word), 
                                        TokenValueType::OPERATION, line, posStart));

    return pos;
}

static size_t ParseExclamation(const char* str, const size_t posStart, const size_t line, 
                                                                        TokensArrType* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    static const size_t  wordLength  =  64;
    static char     word[wordLength] =  "";

    word[0] = str[pos];
    pos++;

    if (str[pos] == '=')
    {
        word[1] = '=';
        pos++;
        word[2] = '\0';
    }
    else
    {
        assert(false);

        //TODO: syn_assert / or add not as !
    }

    VectorPush(tokens, TokenCreate(TokenValueCreateOp(word),    
                                        TokenValueType::OPERATION, line, posStart));

    return pos;
}

static size_t ParseLessOrGreater(const char* str, const size_t posStart, const size_t line, 
                                                                        TokensArrType* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    static const size_t  wordLength  =  3;
    static char     word[wordLength] = "";

    word[0] = str[pos];
    pos++;

    word[1] = '\0';

    if (str[pos] == '=')
    {
        word[1] = '=';
        pos++;
        word[2] = '\0';
    }

    VectorPush(tokens, TokenCreate(TokenValueCreateOp(word), 
                                        TokenValueType::OPERATION, line, posStart));

    return pos;
}

static size_t Parse5(const char* str, const size_t posStart, const size_t line, 
                                                                    TokensArrType* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    int value = 0;
    int shift = 0;
    sscanf(str + pos, "%d%n", &value, &shift);

    pos += shift;

    if (value != 57 && value != 5757)
        return ParseDigit(str, posStart, line, tokens);

    if (value == 5757)
    {
        VectorPush(tokens, TokenCreate(TokenValueCreateOp("5757"), 
                                            TokenValueType::OPERATION, line, pos));

        return pos;
    }

    //Value is 57:
    assert(value == 57);

    if (str[pos] == '?')
    {
        VectorPush(tokens, TokenCreate(TokenValueCreateOp("57?"), 
                                            TokenValueType::OPERATION, line, pos));

        return pos + 1;
    }

    if (str[pos] == '!')
    {
        VectorPush(tokens, TokenCreate(TokenValueCreateOp("57!"), 
                                            TokenValueType::OPERATION, line, pos));

        return pos + 1;
    }

    VectorPush(tokens, TokenCreate(TokenValueCreateWord("57"),
                                            TokenValueType::KEY_WORD, line, pos));

    return pos;
}

TreeType CodeParse(const char* str)
{
    assert(str);

    TreeType expression = {};

    DescentStorage storage = {};
    DescentStorageCtor(&storage);

    ParseOnTokens(str, &storage.tokens);
    //expression.root = GetG(&storage);

    //DescentStorageDtor(&storage);
    //TreeGraphicDump(&expression, true);
    return expression;
}

static TreeErrors ParseOnTokens(const char* str, TokensArrType* tokens)
{
    size_t pos  = 0;
    size_t line = 0;

    while (str[pos] != '\0')
    {
        switch (str[pos])
        {
            case '+':
            case '*':
            case '/':
            case '^':
            {
                pos = ParseChar(str, pos, line, tokens);
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
                pos = ParseExclamation(str, pos, line, tokens);
                break;
            }

            case '-':
            {
                //TODO: rename create_word / create_op, not obvious that word is for not op's like
                VectorPush(tokens, TokenCreate(TokenValueCreateWord("-"), 
                                                    TokenValueType::KEY_WORD, line, pos));
                ++pos;
                break;
            }

            case '(':
            {
                //TODO: пока что operation, при добавлении новых смыслов -> word
                VectorPush(tokens, TokenCreate(TokenValueCreateOp("("), 
                                                    TokenValueType::OPERATION, line, pos));
                ++pos;
                break;
            }

            case ')':
            {
                VectorPush(tokens, TokenCreate(TokenValueCreateOp(")"), 
                                                    TokenValueType::OPERATION, line, pos));
                ++pos;
                break;
            }

            case '\t':
            case ' ':
            {
                const char* strPtr = SkipSymbolsWhileStatement(str + pos, isspace);
                pos = strPtr - str;
                break;
            }
            case '\n':
            {
                const char* strPtr = SkipSymbolsWhileStatement(str + pos, isspace);
                pos = strPtr - str;
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

                printf("char - %c, string - %s\n", str[pos], str);
                //TODO: здесь syn_assert очев, такого не бывает же
                assert(0 == 1);
                break;
            }
        }
    }

    for (size_t i = 0; i < tokens->size; ++i)
    {
        switch (tokens->data[i].valueType)
        {
            case TokenValueType::OPERATION:
                printf("Operation - %d, %s\n", tokens->data[i].value.operation, 
                     TreeOperationGetLongName(tokens->data[i].value.operation));
                break;
            case TokenValueType::VARIABLE:
                printf("Variable - %s\n", tokens->data[i].value.word);
                break;
            case TokenValueType::VALUE:
                printf("Value - %lf\n", tokens->data[i].value.val);
                break;
            case TokenValueType::KEY_WORD:
                printf("Key word - %s\n", tokens->data[i].value.word);
                break;
            default:
                abort();
                break;
        }
    }
    
    VectorPush(tokens, TokenCreate(TokenValueCreateOp("\0"), TokenValueType::OPERATION, 
                                                                            line, pos));

    return TreeErrors::NO_ERR;
}

TokenType TokenCreate(TokenValue value, TokenValueType valueType,   const size_t line, 
                                                                    const size_t pos)
{
    TokenType token = {};

    token.value     = value;
    token.valueType = valueType;
    token.line      =      line;
    token.pos       =       pos;

    return token;
}

TokenType TokenCopy(const TokenType* token)
{
    return TokenCreate(token->value, token->valueType, token->line, token->pos);
}

TokenValue TokenValueCreateNum(double value)
{
    TokenValue val =
    {
        .val = value,
    };

    return val;
}

TokenValue TokenValueCreateWord(const char* word)
{
    TokenValue val =
    {
        .word = strdup(word),
    };

    return val;
}

TokenValue TokenValueCreateOp(const char* word)
{
    int id = TreeOperationGetId(word);

    assert(id >= 0);

    TokenValue val =
    {
        .operation = (TreeOperationId)id,
    };

    return val;
}

//-------------------Recursive descent-----------------

#define SyntaxAssert(statement) SynAssert(statement, __FILE__, __func__, __LINE__)
static inline void SynAssert(bool statement, const char* fileName, const char* funcName, const int line) 
                                //const char* string, const size_t line, const size_t pos)
{
    if (statement)
        return;

    printf("File - %s, func - %s, line - %d\n", fileName, funcName, line);

    assert(false);
    //assert(string);

    //printf(RED_TEXT("Syntax error in line %zu, pos %zu, string - %s"), line, pos, string);
}

static void DescentStorageCtor(DescentStorage* storage)
{
    VectorCtor(&storage->tokens);
    storage->tokenPos  = 0;
}

static void DescentStorageDtor(DescentStorage* storage)
{
    VectorDtor(&storage->tokens);
    storage->tokenPos = 0;
}
