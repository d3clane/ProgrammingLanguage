#include <ctype.h>
#include <assert.h>
#include <string.h>

#include "Tree/DSL.h"
#include "Tree/Tree.h"
#include "Parser.h"
#include "Common/StringFuncs.h"
#include "Vector/Vector.h"
#include "Common/Colors.h"
#include "Common/Log.h"

typedef VectorType TokensArrType;

static TreeErrors ParseOnTokens(const char* str, TokensArrType* tokens);

//TODO: ХРАНЮ указатель на неймтейбл локальных переменных внутри глобального нейм тейбла функции 

struct DescentStorage
{
    TokensArrType tokens;

    size_t tokenPos;

    NameTableType globalTable;
    NameTableType currentLocalTable;
};

enum class AddVar
{
    ADD_TO_GLOBAL,
    ADD_TO_LOCAL,
    DONT_ADD,
};

static void DescentStorageCtor(DescentStorage* storage);
static void DescentStorageDtor(DescentStorage* storage);

#define T_CRT_NUM(val)  TokenValueCreateNum(val)
#define T_CRT_VAR(val)  TokenValueCreateWord(word)
#define T_CRT_OP(val)   TokenValueCreateOp(word)

#define     T_OP_TYPE_CNST TokenValueType::OPERATION
#define    T_NUM_TYPE_CNST TokenValueType::VALUE
#define    T_VAR_TYPE_CNST TokenValueType::VARIABLE
#define T_K_WORD_TYPE_CNST TokenValueType::KEY_WORD

#define POS(storage) storage->tokenPos

static inline TreeNodeType* GetVar          (DescentStorage* storage, bool* outErr, AddVar addVarEnum);
static inline TreeNodeType* GetG            (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetAddSub       (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetType         (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncDef      (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFunc         (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncVarsDef  (DescentStorage* storage, bool* outErr, AddVar addvarEnum);
static inline TreeNodeType* GetPrint        (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetRead         (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetVarDef       (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncCall     (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncVarCall (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetWhile        (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetIf           (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetOp           (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetAssign       (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetAnd          (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetOr           (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetCmp          (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetMulDiv       (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetPow          (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetPrefixFunc   (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetExpr         (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetArg          (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetNum          (DescentStorage* storage, bool* outErr);


static inline TokenType* T_LAST_TOKEN(const DescentStorage* storage)
{
    return &storage->tokens.data[storage->tokenPos];
}

static inline const char* T_WORD(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.word;
}

static inline const char* T_WORD(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].value.word;
}

static inline TreeOperationId T_OP(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.operation;
}

static inline TreeOperationId T_OP(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].value.operation;
}

static inline int T_NUM(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.val;
}

static inline int T_NUM(const DescentStorage* storage, const size_t pos)
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

static inline bool T_CMP_OP(const DescentStorage* storage, TreeOperationId operation)
{
    return T_IS_OP(storage) && T_OP(storage) == operation;
}

static inline bool T_CMP_OP(const DescentStorage* storage, const size_t pos, 
                            TreeOperationId operation)
{
    return T_IS_OP(storage, pos) && T_OP(storage, pos) == operation;
}

static inline bool T_IS_K_WORD(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].valueType == T_K_WORD_TYPE_CNST;
}

static inline bool T_IS_K_WORD(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == T_K_WORD_TYPE_CNST;
}

static inline bool T_IS_NUM(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].valueType == T_NUM_TYPE_CNST;
}

static inline bool T_IS_NUM(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == T_NUM_TYPE_CNST;
}

static inline bool T_CMP_NUM(const DescentStorage* storage, int value)
{
    return T_IS_NUM(storage) && T_NUM(storage) == value;
}

static inline bool T_CMP_NUM(const DescentStorage* storage, const size_t pos, int value)
{
    return T_IS_NUM(storage, pos) && T_NUM(storage, pos) == value;
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
    return (T_IS_VAR(storage) || T_IS_K_WORD(storage)) && (strcmp(T_WORD(storage), str) == 0);
}

static inline bool T_CMP_WORD(const DescentStorage* storage, const size_t pos, const char* str)
{
    return (T_IS_VAR(storage, pos) || T_IS_K_WORD(storage, pos)) && 
            (strcmp(T_WORD(storage, pos), str) == 0);
}

static inline bool T_CMP_WORD(const TokensArrType* tokens, const size_t pos, const char* str)
{
    //TODO: 
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

    if (value != 57 && value != 5757 && value != 575757)
        return ParseDigit(str, posStart, line, tokens);

    if (value == 575757)
    {
        VectorPush(tokens, TokenCreate(TokenValueCreateOp("575757"), 
                                            TokenValueType::OPERATION, line, posStart));

        return pos;  
    }

    if (value == 5757)
    {
        VectorPush(tokens, TokenCreate(TokenValueCreateOp("5757"), 
                                            TokenValueType::OPERATION, line, posStart));

        return pos;
    }

    //Value is 57:
    assert(value == 57);

    if (str[pos] == '?')
    {
        pos++;
        if (str[pos] == '?')
        {
            VectorPush(tokens, TokenCreate(TokenValueCreateOp("57??"), 
                                                TokenValueType::OPERATION, line, posStart)); 
            pos++;           
        }
        else
            VectorPush(tokens, TokenCreate(TokenValueCreateOp("57?"), 
                                                TokenValueType::OPERATION, line, posStart));

        return pos;
    }

    if (str[pos] == '!')
    {
        pos++;
        if (str[pos] == '!')
        {
            VectorPush(tokens, TokenCreate(TokenValueCreateOp("57!!"), 
                                                TokenValueType::OPERATION, line, posStart));
            pos++;
        }
        else
            VectorPush(tokens, TokenCreate(TokenValueCreateOp("57!"), 
                                    TokenValueType::OPERATION, line, posStart));

        return pos;
    }

    VectorPush(tokens, TokenCreate(TokenValueCreateWord("57"),
                                            TokenValueType::KEY_WORD, line, posStart));

    return pos;
}

TreeType CodeParse(const char* str)
{
    assert(str);

    TreeType expression = {};

    DescentStorage storage = {};
    DescentStorageCtor(&storage);

    ParseOnTokens(str, &storage.tokens);

    bool err = false;
    expression.root = GetG(&storage, &err);

    printf(GREEN_TEXT("global table size - %zu\n"), storage.globalTable.size);
    for (size_t i = 0; i < storage.globalTable.size; ++i)
    {
        printf(RED_TEXT("Global table val - %s\n"), storage.globalTable.data[i].name);
    }

    TreeGraphicDump(&expression, true, &storage.globalTable);

    DescentStorageDtor(&storage);
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
                VectorPush(tokens, TokenCreate(TokenValueCreateOp("-"), 
                                                    TokenValueType::OPERATION, line, pos));
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

                //TODO: здесь syn_assert очев, такого не бывает же
                assert(0 == 1);
                break;
            }
        }
    }

    for (size_t i = 0; i < tokens->size; ++i)
    {
        printf("--------------------------------\n");
        printf("line - %zu, pos - %zu\n", tokens->data[i].line, tokens->data[i].pos);
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
                printf("Value - %d\n", tokens->data[i].value.val);
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

TokenValue TokenValueCreateNum(int value)
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

#define SynAssert(storage, statement, outErr)               \
do                                                          \
{                                                           \
    assert(outErr);                                         \
    SYN_ASSERT(storage, statement, outErr);                 \
    printf("func - %s, line - %d\n", __func__, __LINE__);   \
    LOG_BEGIN();                                            \
    Log("func - %s, line - %d\n", __func__, __LINE__);      \
    LOG_END();                                              \
} while (0)

#define IF_ERR_RET(outErr, node1, node2)                \
do                                                      \
{                                                       \
    if (*outErr)                                        \
    {                                                   \
        if (node1) TreeNodeDeepDtor(node1);             \
        if (node2) TreeNodeDeepDtor(node2);             \
        return nullptr;                                 \
    }                                                   \
} while (0)

static inline void SYN_ASSERT(DescentStorage* storage, bool statement, bool* outErr)
{
    assert(storage);
    assert(outErr);

    if (statement)
        return;

    printf(RED_TEXT("Syntax error in line %zu, pos %zu\n"), T_LAST_TOKEN(storage)->line,
                                                            T_LAST_TOKEN(storage)->pos);
    *outErr = true;
}



static inline TreeNodeType* GetG(DescentStorage* storage, bool* outErr)
{
    assert(storage);
    assert(outErr);

    TreeNodeType* root = GetFunc(storage, outErr);
    IF_ERR_RET(outErr, root, nullptr);

    while (!T_CMP_OP(storage, TreeOperationId::PROGRAMM_END))
    {
        TreeNodeType* tmpNode = GetFunc(storage, outErr);
        IF_ERR_RET(outErr, root, tmpNode);

        root = _NF(root, tmpNode);
    }

    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::PROGRAMM_END), outErr);
    IF_ERR_RET(outErr, root, nullptr);

    return root;
}

static inline TreeNodeType* GetFunc(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* node = GetFuncDef(storage, outErr);
    IF_ERR_RET(outErr, node, nullptr);

    return node;
}

static inline TreeNodeType* GetFuncDef(DescentStorage* storage, bool* outErr)
{
    NameTableType localNameTable = {};
    NameTableCtor(&localNameTable);

    TreeNodeType* func = nullptr;

    TreeNodeType* typeNode = GetType(storage, outErr);
    IF_ERR_RET(outErr, typeNode, nullptr);

    TreeNodeType* funcName = GetVar(storage, outErr, AddVar::ADD_TO_GLOBAL);
    IF_ERR_RET(outErr, typeNode, funcName);
    if (!funcName) return nullptr;
    
    //TODO: create set table function maybe
    storage->globalTable.data[funcName->value.varId].localNameTable = (void*)&localNameTable;

    func = _FUNC(funcName);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    POS(storage)++;

    TreeNodeType* funcVars = GetFuncVarsDef(storage, outErr, AddVar::ADD_TO_LOCAL);
    funcName->left = funcVars;
    IF_ERR_RET(outErr, func, typeNode);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    POS(storage)++;

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, func, typeNode); //Checking for function code start
    
    TreeNodeType* funcCode = GetOp(storage, outErr);
    func->right = funcCode;
    IF_ERR_RET(outErr, func, typeNode);

    /*SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "5757"), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    POS(storage)++;*/ 
    // OP HAS ALREADY CHECKED AND MOVED

    func = _TYPE(typeNode, func);

    return func;
}

/*static inline TreeNodeType* GetFuncDecl(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* func = nullptr;

    TreeNodeType* typeNode = GetType(storage, outErr);
    IF_ERR_RET(outErr, typeNode, nullptr);

    TreeNodeType* funcName = GetVar(storage, outErr, AddVar::ADD_TO_GLOBAL);
    IF_ERR_RET(outErr, typeNode, funcName);

    func = _FUNC(funcName);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    POS(storage)++;

    TreeNodeType* funcVars = GetFuncVarsDef(storage, outErr, AddVar::DONT_ADD);
    funcName->left = funcVars;
    IF_ERR_RET(outErr, func, typeNode);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    POS(storage)++;

    func = _TYPE(typeNode, func);

    return func;
}*/

static inline TreeNodeType* GetType(DescentStorage* storage, bool* outErr)
{
    if (T_CMP_OP(storage, TreeOperationId::TYPE_INT))
    {
        POS(storage)++;
        return TreeNodeCreate(TreeNodeOpValueCreate(TreeOperationId::TYPE_INT), 
                              TreeNodeValueTypeof::OPERATION);
    }

    SynAssert(storage, false, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    return nullptr;
}

static inline TreeNodeType* GetFuncVarsDef(DescentStorage* storage, bool* outErr,
                                                                    AddVar addVarEnum)
{
    TreeNodeType* varsDefNode = nullptr;
        
    if (!T_CMP_OP(storage, TreeOperationId::TYPE_INT))
        return nullptr;
    
    TreeNodeType* varType = GetType(storage, outErr);
    IF_ERR_RET(outErr, varType, nullptr);
    
    TreeNodeType* varName = GetVar(storage, outErr, addVarEnum);
    IF_ERR_RET(outErr, varType, varName);

    varsDefNode = _TYPE(varType, varName);

    while (!(T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57")))
    {
        varType = GetType(storage, outErr);
        IF_ERR_RET(outErr, varsDefNode, varType);

        varName = GetVar(storage, outErr, addVarEnum);
        TreeNodeType* tmpVar = _TYPE(varType, varName);
        IF_ERR_RET(outErr, tmpVar, varsDefNode);

        varsDefNode = _COMMA(varsDefNode, tmpVar);
    }

    return varsDefNode;
}

static inline TreeNodeType* GetOp(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* opNode = nullptr;
    printf("In op\n");
    
    printf("T is op - %d\n", T_IS_OP(storage));
    
    if (T_IS_OP(storage))
    {
        printf("p id - %d\n", (int)T_OP(storage));
    }
    if (T_IS_OP(storage, storage->tokenPos + 1))
    {
        printf("next p id - %d\n", (int)T_OP(storage, storage->tokenPos + 1));
    }


    if (T_CMP_OP(storage, TreeOperationId::IF))
    {
        opNode = GetIf(storage, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        return opNode;
    }
    else if (T_CMP_OP(storage, TreeOperationId::WHILE))
    {
        opNode = GetWhile(storage, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        return opNode;
    }
    else if (T_CMP_OP(storage, TreeOperationId::PRINT))
        opNode = GetPrint(storage, outErr);
    else if (T_CMP_OP(storage, TreeOperationId::READ))
        opNode = GetRead(storage, outErr);
    else if (T_CMP_OP(storage, TreeOperationId::TYPE_INT))
        opNode = GetVarDef(storage, outErr);
    else if (T_CMP_OP(storage, storage->tokenPos + 1, TreeOperationId::ASSIGN))
        opNode = GetAssign(storage, outErr);
    else if (T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"))
    {
        POS(storage)++;

        TreeNodeType* jointNode = _LINE_END(nullptr, nullptr);
        opNode = jointNode;
        while (true)
        {
            printf(RED_TEXT("Going in op recursively\n"));
            TreeNodeType* opInNode = GetOp(storage, outErr);
            IF_ERR_RET(outErr, opNode, opInNode);

            jointNode->left = opInNode;

            if (T_CMP_OP(storage, TreeOperationId::BLOCK_END))
                break;

            jointNode->right = _LINE_END(nullptr, nullptr);
            jointNode        = jointNode->right;
        }

        printf(GREEN_TEXT("OUT OF RECURSION\n"));

        SynAssert(storage, T_CMP_OP(storage, TreeOperationId::BLOCK_END), outErr);
        IF_ERR_RET(outErr, opNode, nullptr);
        POS(storage)++;

        return opNode;
    }
    else
        opNode = GetFuncCall(storage, outErr);

    IF_ERR_RET(outErr, opNode, nullptr);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, opNode, nullptr);
    POS(storage)++;

    return opNode;
}

static inline TreeNodeType* GetIf(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::IF), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* condition = GetOr(storage, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, condition, nullptr);
    POS(storage)++;

    TreeNodeType* op = GetOp(storage, outErr);
    IF_ERR_RET(outErr, condition, op);

    return _IF(condition, op);
}

static inline TreeNodeType* GetWhile(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::WHILE), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* condition = GetOr(storage, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, condition, nullptr);
    POS(storage)++;

    TreeNodeType* op = GetOp(storage, outErr);
    IF_ERR_RET(outErr, condition, op);

    return _WHILE(condition, op);
}

static inline TreeNodeType* GetVarDef(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* typeNode = GetType(storage, outErr);
    IF_ERR_RET(outErr, typeNode, nullptr);

    TreeNodeType* varName = GetVar(storage, outErr, AddVar::ADD_TO_LOCAL);
    IF_ERR_RET(outErr, typeNode, varName);

    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::ASSIGN), outErr);
    IF_ERR_RET(outErr, typeNode, varName);
    POS(storage)++;

    TreeNodeType* expr = GetExpr(storage, outErr);
    TreeNodeType* assign = _ASSIGN(varName, expr);
    IF_ERR_RET(outErr, expr, typeNode);

    return _TYPE(typeNode, assign);
}

static inline TreeNodeType* GetExpr(DescentStorage* storage, bool* outErr)
{
    if (!T_CMP_OP(storage, TreeOperationId::L_BRACKET))
        return GetArg(storage, outErr);

    POS(storage)++;
    
    TreeNodeType* inBracketsExpr = GetOr(storage, outErr);
    IF_ERR_RET(outErr, inBracketsExpr, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::R_BRACKET), outErr);
    IF_ERR_RET(outErr, inBracketsExpr, nullptr);

    return inBracketsExpr;
}

static inline TreeNodeType* GetPrint(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::PRINT), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* arg = GetArg(storage, outErr);
    IF_ERR_RET(outErr, arg, nullptr);

    printf("HERE\n");

    return _PRINT(arg);
}

static inline TreeNodeType* GetRead(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::READ), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* arg = GetArg(storage, outErr);
    IF_ERR_RET(outErr, arg, nullptr);

    return _PRINT(arg);
}

static inline TreeNodeType* GetAssign(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* var = GetVar(storage, outErr, AddVar::DONT_ADD);
    IF_ERR_RET(outErr, var, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::ASSIGN), outErr);
    IF_ERR_RET(outErr, var, nullptr);
    POS(storage)++;

    TreeNodeType* rightExpr = GetOr(storage, outErr);
    IF_ERR_RET(outErr, rightExpr, var);

    return _ASSIGN(var, rightExpr);
}

static inline TreeNodeType* GetFuncCall(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* funcName = GetVar(storage, outErr, AddVar::DONT_ADD);
    IF_ERR_RET(outErr, funcName, nullptr);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, funcName, nullptr);
    POS(storage)++;

    TreeNodeType* funcVars = GetFuncVarCall(storage, outErr);
    IF_ERR_RET(outErr, funcName, funcVars);

    SynAssert(storage, T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"), outErr);
    IF_ERR_RET(outErr, funcName, funcVars);
    POS(storage)++;

    return _FUNC_CALL(funcName, funcVars);
}

static inline TreeNodeType* GetFuncVarCall(DescentStorage* storage, bool* outErr)
{
    if (T_IS_K_WORD(storage) && T_CMP_WORD(storage, "57"))
        return nullptr;
    
    TreeNodeType* vars = GetArg(storage, outErr);
    IF_ERR_RET(outErr, vars, nullptr);

    while (T_IS_NUM(storage) || T_IS_VAR(storage))
    {
        TreeNodeType* tmpVar = GetArg(storage, outErr);
        IF_ERR_RET(outErr, vars, tmpVar);

        vars = _COMMA(vars, tmpVar);
    }
    
    return vars;
}

static inline TreeNodeType* GetOr(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetAnd(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (T_CMP_OP(storage, TreeOperationId::OR))
    {
        POS(storage)++;

        TreeNodeType* tmpExpr = GetAnd(storage, outErr);
        IF_ERR_RET(outErr, tmpExpr, allExpr);

        allExpr = _OR(allExpr, tmpExpr);
    }

    return allExpr;
}

static inline TreeNodeType* GetAnd(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetCmp(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (T_CMP_OP(storage, TreeOperationId::AND))
    {
        POS(storage)++;

        TreeNodeType* tmpExpr = GetCmp(storage, outErr);
        IF_ERR_RET(outErr, tmpExpr, allExpr);

        allExpr = _AND(allExpr, tmpExpr);
    }

    return allExpr;
}

static inline int GetCmpOp(DescentStorage* storage)
{
    int myOp = -1;

    #define GENERATE_CMD(NAME)                          \
        if (T_CMP_OP(storage, TreeOperationId::NAME))   \
            myOp = (int)TreeOperationId::NAME;          \
        else
    
    GENERATE_CMD(LESS)
    GENERATE_CMD(LESS_EQ)
    GENERATE_CMD(GREATER)
    GENERATE_CMD(GREATER_EQ)
    GENERATE_CMD(EQ)
    GENERATE_CMD(NOT_EQ)

    /* else */
    {
        myOp = -1;
    }

    #undef GENERATE_CMD

    return myOp;
}

static inline TreeNodeType* GetCmp(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetAddSub(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    int myOp = GetCmpOp(storage);

    while (myOp != -1)
    {
        POS(storage)++;

        TreeNodeType* newExpr = GetAddSub(storage, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        #define GENERATE_CMD(NAME)                      \
            if (myOp == (int)TreeOperationId::NAME)     \
                allExpr = _##NAME(allExpr, newExpr);    \
            else 
        
        GENERATE_CMD(LESS)
        GENERATE_CMD(LESS_EQ)
        GENERATE_CMD(GREATER)
        GENERATE_CMD(GREATER_EQ)
        GENERATE_CMD(EQ)
        GENERATE_CMD(NOT_EQ)

        /* else */
        {
            SynAssert(storage, false, outErr);
            IF_ERR_RET(outErr, allExpr, newExpr);
        }

        #undef GENERATE_CMD

        myOp = GetCmpOp(storage);
    }

    return allExpr;
}

static inline int GetAddSubOp(DescentStorage* storage)
{
    int myOp = -1;

    if (T_CMP_OP(storage, TreeOperationId::ADD))
        myOp = (int)TreeOperationId::ADD;
    else if (T_CMP_OP(storage, TreeOperationId::SUB))
        myOp = (int)TreeOperationId::SUB;
    else
        myOp = -1;
    
    return myOp;
}

static inline TreeNodeType* GetAddSub(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetMulDiv(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    int myOp = GetAddSubOp(storage);

    while (myOp != -1)
    {
        POS(storage)++;

        TreeNodeType* newExpr = GetMulDiv(storage, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        if (myOp == (int)TreeOperationId::ADD)
            allExpr = _ADD(allExpr, newExpr);
        else if (myOp == (int)TreeOperationId::SUB)
            allExpr = _SUB(allExpr, newExpr);
        else 
        {
            SynAssert(storage, false, outErr);
            IF_ERR_RET(outErr, allExpr, newExpr);
        }

        myOp = GetAddSubOp(storage);
    }

    return allExpr;
}

static inline int GetMulDivOp(DescentStorage* storage)
{
    int myOp = -1;

    if (T_CMP_OP(storage, TreeOperationId::MUL))
        myOp = (int)TreeOperationId::MUL;
    else if (T_CMP_OP(storage, TreeOperationId::DIV))
        myOp = (int)TreeOperationId::DIV;
    else
        myOp = -1;
    
    return myOp;
}

static inline TreeNodeType* GetMulDiv(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetPow(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    int myOp = GetMulDivOp(storage);

    while (myOp != -1)
    {
        POS(storage)++;

        TreeNodeType* newExpr = GetPow(storage, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        if (myOp == (int)TreeOperationId::MUL)
            allExpr = _MUL(allExpr, newExpr);
        else if (myOp == (int)TreeOperationId::DIV)
            allExpr = _DIV(allExpr, newExpr);
        else 
        {
            SynAssert(storage, false, outErr);
            IF_ERR_RET(outErr, allExpr, newExpr);
        }

        myOp = GetMulDivOp(storage);
    }

    return allExpr;
}

static inline int GetPowOp(DescentStorage* storage)
{
    int myOp = -1;

    if (T_CMP_OP(storage, TreeOperationId::POW))
        myOp = (int)TreeOperationId::POW;
    else
        myOp = -1;
    
    return myOp;
}

static inline TreeNodeType* GetPow(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetPrefixFunc(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    int myOp = GetPowOp(storage);

    while (myOp != -1)
    {
        POS(storage)++;

        TreeNodeType* newExpr = GetPrefixFunc(storage, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        if (myOp == (int)TreeOperationId::POW)
            allExpr = _POW(allExpr, newExpr);
        else 
        {
            SynAssert(storage, false, outErr);
            IF_ERR_RET(outErr, allExpr, newExpr);
        }

        myOp = GetPowOp(storage);
    }

    return allExpr;
}

static inline int GetPrefixFuncOp(DescentStorage* storage)
{
    int myOp = -1;

    #define GENERATE_CMD(NAME)                          \
        if (T_CMP_OP(storage, TreeOperationId::NAME))   \
            myOp = (int)TreeOperationId::NAME;          \
        else
    
    GENERATE_CMD(SIN)
    GENERATE_CMD(COS)
    GENERATE_CMD(TAN)
    GENERATE_CMD(COT)
    GENERATE_CMD(SQRT)

    /* else */
    {
        myOp = -1;
    }

    #undef GENERATE_CMD

    return myOp;
}

static inline TreeNodeType* GetPrefixFunc(DescentStorage* storage, bool* outErr)
{
    int myOp = GetPrefixFuncOp(storage);

    if (myOp == -1)
        return GetExpr(storage, outErr);

    assert(myOp != -1);

    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::L_BRACKET), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* expr = GetOr(storage, outErr);
    IF_ERR_RET(outErr, expr, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TreeOperationId::R_BRACKET), outErr);
    IF_ERR_RET(outErr, expr, nullptr);
    POS(storage)++;

    #define GENERATE_CMD(NAME)                      \
        if (myOp == (int)TreeOperationId::NAME)     \
            expr = _##NAME(expr);                   \
        else 
    
    GENERATE_CMD(SIN)
    GENERATE_CMD(COS)
    GENERATE_CMD(TAN)
    GENERATE_CMD(COT)
    GENERATE_CMD(SQRT)

    /* else */
    {
        SynAssert(storage, false, outErr);
        IF_ERR_RET(outErr, expr, nullptr);
    }

    #undef GENERATE_CMD    

    return expr;
}

static inline TreeNodeType* GetArg(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* arg = nullptr;

    if (T_IS_NUM(storage))
        arg = GetNum(storage, outErr);
    else 
        arg = GetVar(storage, outErr, AddVar::DONT_ADD);

    IF_ERR_RET(outErr, arg, nullptr);

    return arg;
}

static inline TreeNodeType* GetNum(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_IS_NUM(storage), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    TreeNodeType* num = CRT_NUM(T_NUM(storage));
    POS(storage)++;

    return num;
}

//TODO: сюда передавать localNameTable, тут его сувать(мб)
static inline TreeNodeType* GetVar(DescentStorage* storage, bool* outErr, AddVar addVarEnum)
{
    SynAssert(storage, T_IS_VAR(storage), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    TreeNodeType* varNode = nullptr;
    
    //TODO: создать отдельную функцию createName
    Name pushName = 
    {
        .name           = strdup(T_WORD(storage)),
        .nameType       = NameType::VARIABLE,

        .localNameTable = nullptr,
    };

    //TODO: здесь проверки на то, что мы пушим (в плане того, чтобы не было конфликтов имен и т.д, пока похуй)
    switch (addVarEnum)
    {
        case AddVar::ADD_TO_GLOBAL:
        {
            NameTablePush(&storage->globalTable, pushName);
            varNode = CRT_VAR(storage->globalTable.size);
            break;
        }

        case AddVar::ADD_TO_LOCAL:
        {
            NameTablePush(&storage->currentLocalTable, pushName);
            varNode = CRT_VAR(storage->currentLocalTable.size);
            break;
        }

        case AddVar::DONT_ADD:
        {
            Name* outName = nullptr;
            NameTableFind(&storage->currentLocalTable, T_WORD(storage), &outName);
            
            if (!outName)
                NameTableFind(&storage->currentLocalTable, T_WORD(storage), &outName);
            else
            {
                varNode = CRT_VAR(outName - storage->currentLocalTable.data);
                break;
            }

            SynAssert(storage, outName != nullptr, outErr);
            IF_ERR_RET(outErr, varNode, nullptr);

            varNode = CRT_VAR(outName - storage->globalTable.data);
            break;
        }

        default:
            break;
    }

    POS(storage)++;

    return varNode;
}


static void DescentStorageCtor(DescentStorage* storage)
{
    VectorCtor(&storage->tokens);
    NameTableCtor(&storage->globalTable);
    NameTableCtor(&storage->currentLocalTable);
    storage->tokenPos  = 0;
}

static void DescentStorageDtor(DescentStorage* storage)
{
    VectorDtor(&storage->tokens);

    for (size_t i = 0; i < storage->globalTable.size; ++i)
    {
        if (storage->globalTable.data[i].localNameTable)
        {
            free((NameTableType*)(storage->globalTable.data[i].localNameTable));
            free(storage->globalTable.data[i].name);
        }
    }
    NameTableDtor(&storage->globalTable);
    NameTableDtor(&storage->currentLocalTable);

    storage->tokenPos = 0;
}
