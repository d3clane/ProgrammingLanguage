#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "Tree/Tree.h"
#include "Vector/Vector.h"
#include "ParserCpy.h"
#include "Tree/DSL.h"
#include "Common/StringFuncs.h"

typedef VectorType TokensArrType;


#define T_CRT_VAL(val)  TreeNodeValueCreate(val)
#define  T_OP_TYPE_CNST TokenValueType::OPERATION
#define T_NUM_TYPE_CNST TokenValueType::VALUE
#define T_VAR_TYPE_CNST TokenValueType::VARIABLE

#define POS(storage) storage->tokenPos

/*
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
*/

static size_t ParseDigit(const char* str, const size_t posStart, const size_t line, 
                                                                TokensArrType* tokens)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    int val = 0;
    while (isdigit(str[pos]))
    {
        val = val * 10 + str[pos] - '0';
        ++pos;
    }
    
    TreeNodeValue nodeVal = TreeNodeValueCreate(val);
    VectorPush(tokens, TokenCreate(nodeVal, TreeNodeValueTypeof::VALUE, line, pos));

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

    #define GENERATE_OPERATION_CMD(NAME, v1, v2, v3, SHORT_NAME, ...)                   \
        if (strcmp(word, SHORT_NAME) == 0)                                              \
        {                                                                               \
            VectorPush(tokens,                                                          \
                TokenCreate(T_CRT_VAL(word), TokenValueType::OPERATION,                 \
                                                                line, posStart));       \
            return pos;                                                                 \
        }
    
    //GENERATING if(strcmp(word, "+") == 0) ... if...
    #include "Tree/Operations.h"

    //Now - word is not a special symbol

    VectorPush(tokens, TokenCreate(TreeNodeValueCreate(word), 
                                        TreeNodeValueTypeof::VARIABLE, line, posStart));

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

    TreeNodeValue nodeVal = TreeNodeValueCreate(charWord);
    VectorPush(tokens, TokenCreate(nodeVal, TreeNodeValueTypeof::OPERATION, line, posStart));

    return posStart + 1;
}

/*ExpressionType ExpressionParse(const char* str)
{
    assert(str);

    ExpressionType expression = {};

    DescentStorage storage = {};
    DescentStorageCtor(&storage);

    ParseOnTokens(str, &storage.tokens);
    expression.root = GetG(&storage);

    expression.variables = storage.varsArr;
    //DescentStorageDtor(&storage);
    ExpressionGraphicDump(&expression, true);
    return expression;
}*/

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

    TreeNodeValue nodeVal = TreeNodeValueCreate(word);
    VectorPush(tokens, TokenCreate(nodeVal, TreeNodeValueTypeof::OPERATION, line, posStart));

    return pos;
}

static size_t ParseExclamation(const char* str, const size_t posStart, const size_t line, 
                                TokensArrType* tokens, bool* waitForConditionEnd)
{
    assert(str);
    assert(tokens);
    assert(waitForConditionEnd);

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
    else if (str[pos] == '5')
    {
        int value = 0;
        int shift = 0;
        sscanf(str + pos, "%d%n", &value, &shift);

        if (value == 57)
        {
            word[1] = '5';
            word[2] = '7';
            word[3] = '\0';

            pos += shift;

            *waitForConditionEnd = true;
        }
        else
        {
            assert(false);
            
            //TODO: syn_assert, if "not" added, change
        }
    }
    else
    {
        assert(false);

        //TODO: syn assert
    }

    TreeNodeValue nodeVal = TreeNodeValueCreate(word);
    VectorPush(tokens, TokenCreate(nodeVal, TreeNodeValueTypeof::OPERATION, line, posStart));

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

    TreeNodeValue nodeVal = TreeNodeValueCreate(word);
    VectorPush(tokens, TokenCreate(nodeVal, TreeNodeValueTypeof::OPERATION, line, posStart));

    return pos;
}

static size_t ParseQuestion(const char* str, const size_t posStart, const size_t line, 
                            TokensArrType* tokens, bool* waitForConditionEnd)
{
    assert(str);
    assert(tokens);
    assert(waitForConditionEnd);

    size_t pos = posStart;

    static const size_t  wordLength  =  64;
    static char     word[wordLength] =  "";

    word[0] = str[pos];
    pos++;

    int value = 0;
    int shift = 0;
    sscanf(str + pos, "%d%n", &value, &shift);

    if (value == 57)
    {
        word[1] = '5';
        word[2] = '7';
        word[3] = '\0';

        pos += shift;

        *waitForConditionEnd = true;
    }
    else
    {
        assert(false);
        
        //TODO: syn_assert, if "not" added, change
    }

    TreeNodeValue nodeVal = TreeNodeValueCreate(word);
    VectorPush(tokens, TokenCreate(nodeVal, TreeNodeValueTypeof::OPERATION, line, posStart));

    return pos;
}

static size_t Parse5(const char* str, const size_t posStart, const size_t line, 
                            TokensArrType* tokens, bool waitForConditionEnd)
{
    assert(str);
    assert(tokens);

    size_t pos = posStart;

    static const size_t  wordLength  =  3;
    static char     word[wordLength] = "";

    word[0] = '5';

    int value = 0;

    sscanf(str + posStart, "%d", &value);

    if (value == 57)
    {
        if (waitForConditionEnd)

    }
}

static ParseErrors ParseOnTokens(const char* str, TokensArrType* tokens)
{
    size_t pos  = 0;
    size_t line = 0;

    bool waitForConditionEnd = false;
    while (str[pos] != '\0')
    {
        switch (str[pos])
        {
            // как это можно сделать - я читаю данное мне выражение, вижу -, смотрю в массив токенов 
            // до меня, который я уже построил. Предыдущее должно быть не операцией если это не унарник
            // иначе это унарник, тогда можно считать это как число тип 
            //(удобнее унарник перевести в умножение на (-1) в случае переменной, мне кажется).
            //TODO: +, - могут быть унарными операторами, можно просто пихать прям сразу в число
            case '+':
            case '-':
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
            case '>':
            {
                pos = ParseLessOrGreater(str, pos, line, tokens);
                break;
            }
            case '<':
            {
                pos = ParseLessOrGreater(str, pos, line, tokens);
                break;
            }

            case '!':
            {
                pos = ParseExclamation(str, pos, line, tokens, &waitForConditionEnd);
                break;
            }
            case '?':
            {
                pos = ParseQuestion(str, pos, line, tokens, &waitForConditionEnd);
                break;
            }

            /*case '}':
            {
                VectorPush(tokens, TokenCreate(TreeNodeValueCreate("}"),
                                                TreeNodeValueTypeof::OPERATION, line, pos));
                ++pos;
                break;
            }*/

            case '(':
            {
                VectorPush(tokens, TokenCreate(TreeNodeValueCreate("("), 
                                                TreeNodeValueTypeof::OPERATION, line, pos));
                ++pos;
                break;
            }
            case ')':
            {
                VectorPush(tokens, TokenCreate(TreeNodeValueCreate(")"), 
                                                    TreeNodeValueTypeof::OPERATION, line, pos));
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

            // Unique due to '57' grammar
            case '5':
            {
                pos = Parse5(str, pos, line, tokens, waitForConditionEnd);
                break;
            }
            
            default:
            {
                if (isalpha(str[pos]) || str[pos] == '_')
                {
                    pos = ParseWord(str, pos, line, tokens);
                    break;
                }

                //TODO: здесь syn_assert очев, такого не бывает же
                assert(0 == 1);
                break;
            }
        }
    }

    /*
    for (size_t i = 0; i < tokens->size; ++i)
    {
        switch (tokens->data[i].valueType)
        {
            case TokenValueType::OPERATION:
                printf("Operation - %s\n", tokens->data[i].value.word);
                break;
            case TokenValueType::VARIABLE:
                printf("Variable - %s\n", tokens->data[i].value.word);
                break;
            case TokenValueType::VALUE:
                printf("Value - %lf\n", tokens->data[i].value.val);
                break;
            default:
                abort();
                break;
        }
    }
    */
    
    VectorPush(tokens, TokenCreate(TreeNodeValueCreate("\0"), TreeNodeValueTypeof::OPERATION, 
                                                                                    line, pos));

    return TreeErrors::NO_ERR;
}

TokenType TokenCreate(TreeNodeValue value, TreeNodeValueTypeof valueType, const size_t line, 
                                                                          const size_t pos)
{
    TokenType token = {};

    TreeNodeType* node = TreeNodeCreate(value, valueType);
    token.line      =      line;
    token.pos       =       pos;

    return token;
}

TokenType TokenCopy(const TokenType* token)
{
    return TokenCreate(token->node->value, token->node->valueType, token->line, token->pos);
}
