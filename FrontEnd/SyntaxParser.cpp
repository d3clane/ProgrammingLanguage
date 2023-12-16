#include <ctype.h>
#include <assert.h>
#include <string.h>

#include "LexicalParser.h"
#include "Tree/DSL.h"
#include "Tree/Tree.h"
#include "SyntaxParser.h"
#include "Common/StringFuncs.h"
#include "TokensArr/TokensArr.h"
#include "Common/Colors.h"
#include "Common/Log.h"

struct DescentStorage
{
    TokensArr tokens;

    size_t tokenPos;

    NameTableType* globalTable; 
    //TODO: удалить, всегда добавляем в локалку + отделить функции  
    NameTableType* currentLocalTable;

    NameTableType* allNamesTable;

    const char* codeString;
};

enum class AddVar
{
    ADD_TO_GLOBAL,
    ADD_TO_LOCAL,
    DONT_ADD,
};

static void DescentStorageCtor(DescentStorage* storage, const char* codeString);
static void DescentStorageDtor(DescentStorage* storage);

#define POS(storage) storage->tokenPos

// G                ::= FUNC+ '\0'
// FUNC             ::= FUNC_DEF
// FUNC_DEF         ::= TYPE VAR FUNC_VARS_DEF '57' OP '}' CHANGE!!!
// FUNC_VAR_DEF     ::= {TYPE VAR}*
// OP               ::= {IF | WHILE | '57' OP+ '{'} | { {VAR_DEF | PRINT | ASSIGN | RET} '57' }
// IF               ::= '57?' OR '57' OP
// WHILE            ::= '57!' OR '57' OP
// RET              ::= OR CHANGE!!!!
// VAR_DEF          ::= TYPE VAR '==' OR CHANGE!!!!!
// PRINT            ::= '{' ARG 
// READ             ::= '{'
// ASSIGN           ::= VAR '==' OR
// OR               ::= AND {and AND}*
// AND              ::= CMP {or CMP}*
// CMP              ::= ADD_SUB {[<, <=, >, >=, =, !=] ADD_SUB}*
// ADD_SUB          ::= MUL_DIV {[+, -] MUL_DIV}*
// MUL_DIV          ::= POW {[*, /] POW}*
// POW              ::= FUNC_CALL {['^'] FUNC_CALL}*
// FUNC_CALL        ::= IN_BUILD_FUNCS | CREATED_FUNCS | EXPR
// IN_BUILT_FUNCS   ::= [sin/cos/tan/cot/sqrt] OR '57' | READ
// MADE_FUNC_CALL   ::= VAR '{' FUNC_VARS_CALL '57' 
// FUNC_VARS_CALL   ::= {OR}*
// EXPR             ::= '(' OR ')' | ARG
// ARG              ::= NUM | GET_VAR
// NUM              ::= ['0'-'9']+
// VAR              ::= ['a'-'z' 'A'-'Z' '_']+ ['a'-'z' 'A'-'Z' '_' '0'-'9']*

static inline TreeNodeType* GetVar              (DescentStorage* storage, bool* outErr, AddVar addVarEnum);
static inline TreeNodeType* GetGrammar          (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetAddSub           (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetType             (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncDef          (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFunc             (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncVarsDef      (DescentStorage* storage, bool* outErr, AddVar addvarEnum);
static inline TreeNodeType* GetPrint            (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetRead             (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetVarDef           (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncCall         (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetFuncVarsCall     (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetWhile            (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetIf               (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetOp               (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetAssign           (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetAnd              (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetOr               (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetCmp              (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetMulDiv           (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetPow              (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetMadeFuncCall     (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetInBuiltFuncCall  (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetExpr             (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetArg              (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetNum              (DescentStorage* storage, bool* outErr);
static inline TreeNodeType* GetReturn           (DescentStorage* storage, bool* outErr);

static inline Token* T_LAST_TOKEN(const DescentStorage* storage)
{
    return &storage->tokens.data[storage->tokenPos];
}

static inline const char* T_WORD(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.name;
}

static inline const char* T_WORD(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].value.name;
}

static inline TokenId T_OP(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.tokenId;
}

static inline TokenId T_OP(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].value.tokenId;
}

static inline int T_NUM(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].value.num;
}

static inline int T_NUM(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].value.num;
}

static inline bool T_IS_OP(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].valueType == TokenValueType::TOKEN;
}

static inline bool T_IS_OP(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == TokenValueType::TOKEN;
}

static inline bool T_CMP_OP(const DescentStorage* storage, TokenId tokenId)
{
    return T_IS_OP(storage) && T_OP(storage) == tokenId;
}

static inline bool T_CMP_OP(const DescentStorage* storage, const size_t pos, 
                            TokenId tokenId)
{
    return T_IS_OP(storage, pos) && T_OP(storage, pos) == tokenId;
}

static inline bool T_IS_NUM(const DescentStorage* storage)
{
    return storage->tokens.data[storage->tokenPos].valueType == TokenValueType::NUM;
}

static inline bool T_IS_NUM(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == TokenValueType::NUM;
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
    return storage->tokens.data[storage->tokenPos].valueType == TokenValueType::NAME;
}

static inline bool T_IS_VAR(const DescentStorage* storage, const size_t pos)
{
    return storage->tokens.data[pos].valueType == TokenValueType::NAME;
}

static inline bool T_CMP_WORD(const DescentStorage* storage, const char* str)
{
    return T_IS_VAR(storage) && strcmp(T_WORD(storage), str) == 0;
}

static inline bool T_CMP_WORD(const DescentStorage* storage, const size_t pos, const char* str)
{
    return T_IS_VAR(storage, pos) && strcmp(T_WORD(storage, pos), str) == 0;
}

static inline bool T_CMP_WORD(const TokensArr* tokens, const size_t pos, const char* str)
{
    return strcmp(tokens->data[pos].value.name, str) == 0;
}

TreeType CodeParse(const char* str, SyntaxParserErrors* outErr)
{
    assert(str);

    TreeType expression = {};

    DescentStorage storage = {};
    DescentStorageCtor(&storage, str);

    ParseOnTokens(str, &storage.tokens);

    bool err = false;
    expression.root = GetGrammar(&storage, &err);

    if (err)
        *outErr = SyntaxParserErrors::SYNTAX_ERR; 

    if (!err)
        TreeGraphicDump(&expression, true, storage.allNamesTable);

    DescentStorageDtor(&storage);
    
    return expression;
}

//-------------------Recursive descent-----------------

#define SynAssert(storage, statement, outErr)               \
do                                                          \
{                                                           \
    assert(outErr);                                         \
    SYN_ASSERT(storage, statement, outErr);                 \
    /*printf("func - %s, line - %d\n", __func__, __LINE__);*/  \
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



static inline TreeNodeType* GetGrammar(DescentStorage* storage, bool* outErr)
{
    assert(storage);
    assert(outErr);

    TreeNodeType* root = GetFunc(storage, outErr);
    IF_ERR_RET(outErr, root, nullptr);

    while (!T_CMP_OP(storage, TokenId::PROGRAMM_END))
    {
        TreeNodeType* tmpNode = GetFunc(storage, outErr);
        IF_ERR_RET(outErr, root, tmpNode);

        root = _NEW_FUNC(root, tmpNode);
    }

    SynAssert(storage, T_CMP_OP(storage, TokenId::PROGRAMM_END), outErr);
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
    NameTableType* localNameTable = nullptr;
    NameTableCtor(&localNameTable);

    TreeNodeType* func = nullptr;

    TreeNodeType* typeNode = GetType(storage, outErr);
    IF_ERR_RET(outErr, typeNode, nullptr);

    TreeNodeType* funcName = GetVar(storage, outErr, AddVar::ADD_TO_GLOBAL);
    IF_ERR_RET(outErr, typeNode, funcName);
    
    //TODO: create set table function maybe
    storage->globalTable->data[funcName->value.varId].localNameTable = (void*)localNameTable;
    storage->currentLocalTable = localNameTable;
    func = _FUNC(funcName);

    TreeNodeType* funcVars = GetFuncVarsDef(storage, outErr, AddVar::ADD_TO_LOCAL);
    funcName->left = funcVars;
    IF_ERR_RET(outErr, func, typeNode);

    //TODO: consume func instead of 3 code lines 
    SynAssert(storage, T_CMP_OP(storage, TokenId::FIFTY_SEVEN), outErr);
    IF_ERR_RET(outErr, func, typeNode); //Checking for function code start
    
    TreeNodeType* funcCode = GetOp(storage, outErr);
    funcName->right = funcCode;
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

    SynAssert(storage, T_CMP_OP(storage, TokenId::FIFTY_SEVEN), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    POS(storage)++;

    TreeNodeType* funcVars = GetFuncVarsDef(storage, outErr, AddVar::DONT_ADD);
    funcName->left = funcVars;
    IF_ERR_RET(outErr, func, typeNode);

    SynAssert(storage, T_CMP_OP(storage, TokenId::FIFTY_SEVEN), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    POS(storage)++;

    func = _TYPE(typeNode, func);

    return func;
}*/

static inline TreeNodeType* GetType(DescentStorage* storage, bool* outErr)
{
    if (T_CMP_OP(storage, TokenId::TYPE_INT))
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
        
    if (!T_CMP_OP(storage, TokenId::TYPE_INT))
        return nullptr;
    
    TreeNodeType* varType = GetType(storage, outErr);
    IF_ERR_RET(outErr, varType, nullptr);
    
    TreeNodeType* varName = GetVar(storage, outErr, addVarEnum);
    IF_ERR_RET(outErr, varType, varName);

    varsDefNode = _TYPE(varType, varName);

    while (!(T_CMP_OP(storage, TokenId::FIFTY_SEVEN)))
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

    //TODO: peek - check and don't move
    if (T_CMP_OP(storage, TokenId::IF))
    {
        opNode = GetIf(storage, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        return opNode;
    }
    else if (T_CMP_OP(storage, TokenId::WHILE))
    {
        opNode = GetWhile(storage, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        return opNode;
    }
    else if (T_CMP_OP(storage, TokenId::L_BRACE))
        opNode = GetPrint(storage, outErr);
    else if (T_CMP_OP(storage, TokenId::TYPE_INT))
        opNode = GetVarDef(storage, outErr);
    else if (T_CMP_OP(storage, storage->tokenPos + 1, TokenId::ASSIGN))
        opNode = GetAssign(storage, outErr);
    else if (T_CMP_OP(storage, TokenId::FIFTY_SEVEN))
    {
        POS(storage)++;

        TreeNodeType* jointNode = _LINE_END(nullptr, nullptr);
        opNode = jointNode;
        while (true)
        {
            //printf(RED_TEXT("Going in op recursively\n"));
            TreeNodeType* opInNode = GetOp(storage, outErr);
            IF_ERR_RET(outErr, opNode, opInNode);

            jointNode->left = opInNode;

            if (T_CMP_OP(storage, TokenId::L_BRACE) && 
                !T_CMP_OP(storage, storage->tokenPos + 2, TokenId::FIFTY_SEVEN))
                break;

            jointNode->right = _LINE_END(nullptr, nullptr);
            jointNode        = jointNode->right;
        }

        //printf(GREEN_TEXT("OUT OF RECURSION\n"));

        SynAssert(storage, T_CMP_OP(storage, TokenId::L_BRACE), outErr);
        IF_ERR_RET(outErr, opNode, nullptr);
        POS(storage)++;

        return opNode;
    }
    else
        opNode = GetReturn(storage, outErr);

    IF_ERR_RET(outErr, opNode, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TokenId::FIFTY_SEVEN), outErr);
    IF_ERR_RET(outErr, opNode, nullptr);
    POS(storage)++;

    return opNode;
}

static inline TreeNodeType* GetReturn(DescentStorage* storage, bool* outErr)
{
    return _RETURN(GetOr(storage, outErr));
}

static inline TreeNodeType* GetIf(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TokenId::IF), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* condition = GetOr(storage, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TokenId::FIFTY_SEVEN), outErr);
    IF_ERR_RET(outErr, condition, nullptr);
    POS(storage)++;

    TreeNodeType* op = GetOp(storage, outErr);
    IF_ERR_RET(outErr, condition, op);

    return _IF(condition, op);
}

static inline TreeNodeType* GetWhile(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TokenId::WHILE), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* condition = GetOr(storage, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TokenId::FIFTY_SEVEN), outErr);
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

    SynAssert(storage, T_CMP_OP(storage, TokenId::ASSIGN), outErr);
    IF_ERR_RET(outErr, typeNode, varName);
    POS(storage)++;

    TreeNodeType* expr = GetOr(storage, outErr);
    TreeNodeType* assign = _ASSIGN(varName, expr);
    IF_ERR_RET(outErr, expr, typeNode);

    return _TYPE(typeNode, assign);
}

static inline TreeNodeType* GetExpr(DescentStorage* storage, bool* outErr)
{
    if (!T_CMP_OP(storage, TokenId::L_BRACKET))
        return GetArg(storage, outErr);

    POS(storage)++;
    
    TreeNodeType* inBracketsExpr = GetOr(storage, outErr);
    IF_ERR_RET(outErr, inBracketsExpr, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TokenId::R_BRACKET), outErr);
    IF_ERR_RET(outErr, inBracketsExpr, nullptr);

    return inBracketsExpr;
}

static inline TreeNodeType* GetPrint(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TokenId::L_BRACE), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* arg = GetArg(storage, outErr);
    IF_ERR_RET(outErr, arg, nullptr);

    return _PRINT(arg);
}

static inline TreeNodeType* GetRead(DescentStorage* storage, bool* outErr)
{
    SynAssert(storage, T_CMP_OP(storage, TokenId::L_BRACE), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    return _READ(nullptr, nullptr);
}

static inline TreeNodeType* GetAssign(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* var = GetVar(storage, outErr, AddVar::DONT_ADD);
    IF_ERR_RET(outErr, var, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TokenId::ASSIGN), outErr);
    IF_ERR_RET(outErr, var, nullptr);
    POS(storage)++;

    TreeNodeType* rightExpr = GetOr(storage, outErr);
    IF_ERR_RET(outErr, rightExpr, var);

    return _ASSIGN(var, rightExpr);
}

static inline TreeNodeType* GetMadeFuncCall(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* funcName = GetVar(storage, outErr, AddVar::DONT_ADD);
    IF_ERR_RET(outErr, funcName, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TokenId::L_BRACE), outErr);
    IF_ERR_RET(outErr, funcName, nullptr);
    POS(storage)++;

    TreeNodeType* funcVars = GetFuncVarsCall(storage, outErr);
    IF_ERR_RET(outErr, funcName, funcVars);
    funcName->left = funcVars;

    SynAssert(storage, T_CMP_OP(storage, TokenId::FIFTY_SEVEN), outErr);
    IF_ERR_RET(outErr, funcName, funcVars);
    POS(storage)++;

    return _FUNC_CALL(funcName);
}

static inline TreeNodeType* GetFuncVarsCall(DescentStorage* storage, bool* outErr)
{
    if (T_CMP_OP(storage, TokenId::FIFTY_SEVEN))
        return nullptr;
    
    TreeNodeType* vars = GetOr(storage, outErr);
    IF_ERR_RET(outErr, vars, nullptr);

    while (T_IS_NUM(storage) || T_IS_VAR(storage))
    {
        TreeNodeType* tmpVar = GetOr(storage, outErr);
        IF_ERR_RET(outErr, vars, tmpVar);

        vars = _COMMA(vars, tmpVar);
    }
    
    return vars;
}

static inline TreeNodeType* GetOr(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetAnd(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (T_CMP_OP(storage, TokenId::OR))
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

    while (T_CMP_OP(storage, TokenId::AND))
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
        if (T_CMP_OP(storage, TokenId::NAME))   \
            myOp = (int)TokenId::NAME;          \
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
            if (myOp == (int)TokenId::NAME)     \
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

    if (T_CMP_OP(storage, TokenId::ADD))
        myOp = (int)TokenId::ADD;
    else if (T_CMP_OP(storage, TokenId::SUB))
        myOp = (int)TokenId::SUB;
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

        if (myOp == (int)TokenId::ADD)
            allExpr = _ADD(allExpr, newExpr);
        else if (myOp == (int)TokenId::SUB)
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

    if (T_CMP_OP(storage, TokenId::MUL))
        myOp = (int)TokenId::MUL;
    else if (T_CMP_OP(storage, TokenId::DIV))
        myOp = (int)TokenId::DIV;
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

        if (myOp == (int)TokenId::MUL)
            allExpr = _MUL(allExpr, newExpr);
        else if (myOp == (int)TokenId::DIV)
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

    if (T_CMP_OP(storage, TokenId::POW))
        myOp = (int)TokenId::POW;
    else
        myOp = -1;
    
    return myOp;
}

static inline TreeNodeType* GetPow(DescentStorage* storage, bool* outErr)
{
    TreeNodeType* allExpr = GetFuncCall(storage, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    int myOp = GetPowOp(storage);

    while (myOp != -1)
    {
        POS(storage)++;

        TreeNodeType* newExpr = GetFuncCall(storage, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        if (myOp == (int)TokenId::POW)
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

static inline TreeNodeType* GetFuncCall(DescentStorage* storage, bool* outErr)
{
    if (T_CMP_OP(storage, TokenId::SIN)  || T_CMP_OP(storage, TokenId::COS) ||
        T_CMP_OP(storage, TokenId::TAN)  || T_CMP_OP(storage, TokenId::COT) ||
        T_CMP_OP(storage, TokenId::SQRT) || T_CMP_OP(storage, TokenId::L_BRACE))
        return GetInBuiltFuncCall(storage, outErr);

    if (T_CMP_OP(storage, storage->tokenPos + 1, TokenId::L_BRACE))
        return GetMadeFuncCall(storage, outErr);
    
    return GetArg(storage, outErr);
}

static inline int GetInBuiltFuncOp(DescentStorage* storage)
{
    int myOp = -1;

    #define GENERATE_CMD(NAME)                          \
        if (T_CMP_OP(storage, TokenId::NAME))   \
            myOp = (int)TokenId::NAME;          \
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

static inline TreeNodeType* GetInBuiltFuncCall(DescentStorage* storage, bool* outErr)
{
    if (T_CMP_OP(storage, TokenId::L_BRACE))
        return GetRead(storage, outErr);

    int myOp = GetInBuiltFuncOp(storage);

    if (myOp == -1)
        return GetExpr(storage, outErr);

    assert(myOp != -1);

    SynAssert(storage, T_CMP_OP(storage, TokenId::L_BRACKET), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    POS(storage)++;

    TreeNodeType* expr = GetOr(storage, outErr);
    IF_ERR_RET(outErr, expr, nullptr);

    SynAssert(storage, T_CMP_OP(storage, TokenId::R_BRACKET), outErr);
    IF_ERR_RET(outErr, expr, nullptr);
    POS(storage)++;

    #define GENERATE_CMD(NAME)                      \
        if (myOp == (int)TokenId::NAME)     \
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

        .localNameTable = nullptr,
    };

    //TODO: здесь проверки на то, что мы пушим (в плане того, чтобы не было конфликтов имен и т.д, пока похуй)
    switch (addVarEnum)
    {
        case AddVar::ADD_TO_GLOBAL:
        {
            NameTablePush(storage->allNamesTable, pushName);
            NameTablePush(storage->globalTable, pushName);

            varNode = CRT_VAR(storage->allNamesTable->size - 1);
            break;
        }

        case AddVar::ADD_TO_LOCAL:
        {
            NameTablePush(storage->currentLocalTable, pushName);
            NameTablePush(storage->allNamesTable, pushName);
            varNode = CRT_VAR(storage->allNamesTable->size - 1);
            break;
        }

        case AddVar::DONT_ADD:
        {
            Name* outName = nullptr;
            NameTableFind(storage->allNamesTable, T_WORD(storage), &outName);
            //TODO: здесь пройтись по локали + глобали, проверить на существование переменную типо
            varNode = CRT_VAR(outName - storage->allNamesTable->data);
            break;

            SynAssert(storage, outName != nullptr, outErr);
            IF_ERR_RET(outErr, varNode, nullptr);

            varNode = CRT_VAR(outName - storage->globalTable->data);
            break;
        }

        default:
            break;
    }

    POS(storage)++;

    return varNode;
}


static void DescentStorageCtor(DescentStorage* storage, const char* str)
{
    TokensArrCtor(&storage->tokens);
    NameTableCtor(&storage->globalTable);
    NameTableCtor(&storage->allNamesTable);

    storage->codeString       = str;
    storage->tokenPos  = 0;
}

static void DescentStorageDtor(DescentStorage* storage)
{
    
    for (size_t i = 0; i < storage->globalTable->size; ++i)
    {
        if (storage->globalTable->data[i].localNameTable)
            NameTableDtor((NameTableType*)storage->globalTable->data[i].localNameTable);
    }

    NameTableDtor(storage->globalTable);

    TokensArrDtor(&storage->tokens);
    storage->tokenPos = 0;
}
