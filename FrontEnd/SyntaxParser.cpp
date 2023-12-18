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

struct DescentState
{
    TokensArr tokens;

    size_t tokenPos;

    NameTableType* globalTable; 
    NameTableType* currentLocalTable;

    NameTableType* allNamesTable;

    const char* codeString;
};

static void DescentStateCtor(DescentState* state, const char* codeString);
static void DescentStateDtor(DescentState* state);

#define POS(state) state->tokenPos

// G                ::= FUNC+ '\0'
// FUNC             ::= FUNC_DEF
// FUNC_DEF         ::= TYPE VAR FUNC_VARS_DEF '57' OP '{'
// FUNC_VAR_DEF     ::= {TYPE VAR}*
// OP               ::= {IF | WHILE | '57' OP+ '{' | { {VAR_DEF | PRINT | ASSIGN | RET} '57' }
// IF               ::= '57?' OR '57' OP
// WHILE            ::= '57!' OR '57' OP
// RET              ::= OR
// VAR_DEF          ::= TYPE VAR '==' OR
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

static TreeNode* GetVar              (DescentState* state, bool* outErr);
static TreeNode* AddVar              (DescentState* state, bool* outErr);
static TreeNode* GetGrammar          (DescentState* state, bool* outErr);
static TreeNode* GetAddSub           (DescentState* state, bool* outErr);
static TreeNode* GetType             (DescentState* state, bool* outErr);
static TreeNode* GetFuncDef          (DescentState* state, bool* outErr);
static TreeNode* GetFunc             (DescentState* state, bool* outErr);
static TreeNode* GetFuncVarsDef      (DescentState* state, bool* outErr);
static TreeNode* GetPrint            (DescentState* state, bool* outErr);
static TreeNode* GetRead             (DescentState* state, bool* outErr);
static TreeNode* GetVarDef           (DescentState* state, bool* outErr);
static TreeNode* GetFuncCall         (DescentState* state, bool* outErr);
static TreeNode* GetFuncVarsCall     (DescentState* state, bool* outErr);
static TreeNode* GetWhile            (DescentState* state, bool* outErr);
static TreeNode* GetIf               (DescentState* state, bool* outErr);
static TreeNode* GetOp               (DescentState* state, bool* outErr);
static TreeNode* GetAssign           (DescentState* state, bool* outErr);
static TreeNode* GetAnd              (DescentState* state, bool* outErr);
static TreeNode* GetOr               (DescentState* state, bool* outErr);
static TreeNode* GetCmp              (DescentState* state, bool* outErr);
static TreeNode* GetMulDiv           (DescentState* state, bool* outErr);
static TreeNode* GetPow              (DescentState* state, bool* outErr);
static TreeNode* GetMadeFuncCall     (DescentState* state, bool* outErr);
static TreeNode* GetBuiltInFuncCall  (DescentState* state, bool* outErr);
static TreeNode* GetExpr             (DescentState* state, bool* outErr);
static TreeNode* GetArg              (DescentState* state, bool* outErr);
static TreeNode* GetNum              (DescentState* state, bool* outErr);
static TreeNode* GetReturn           (DescentState* state, bool* outErr);

static inline Token* GetLastToken(DescentState* state)
{
    assert(state);

    return &state->tokens.data[POS(state)];
}

#define SynAssert(state, statement, outErr)               \
do                                                          \
{                                                           \
    assert(outErr);                                         \
    SYN_ASSERT(state, statement, outErr);                 \
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

static inline void SYN_ASSERT(DescentState* state, bool statement, bool* outErr)
{
    assert(state);
    assert(outErr);

    if (statement)
        return;

    printf(RED_TEXT("Syntax error in line %zu, pos %zu\n"), GetLastToken(state)->line,
                                                            GetLastToken(state)->pos);
    *outErr = true;
}

static inline bool PickToken(DescentState* state, TokenId tokenId)
{
    assert(state);

    if (state->tokens.data[POS(state)].valueType     == TokenValueType::TOKEN &&
        state->tokens.data[POS(state)].value.tokenId == tokenId)
        return true;
    
    return false;
}

static inline bool PickNum(DescentState* state)
{
    assert(state);

    if (state->tokens.data[POS(state)].valueType == TokenValueType::NUM)
        return true;

    return false;
}

static inline bool PickName(DescentState* state)
{
    assert(state);

    if (state->tokens.data[POS(state)].valueType == TokenValueType::NAME)
        return true;

    return false;
}

static inline bool ConsumeToken(DescentState* state, TokenId tokenId, bool* outErr)
{
    assert(state);

    if (PickToken(state, tokenId))
    {
        POS(state)++;

        return true;
    }

    SynAssert(state, false, outErr);
    *outErr = true;
    return false;
}

static inline bool ConsumeNum(DescentState* state)
{
    assert(state);

    if (PickNum(state))
    {
        POS(state)++;

        return true;
    }

    return false;
}

static inline bool ConsumeName(DescentState* state)
{
    assert(state);

    if (PickName(state))
    {
        POS(state)++;

        return true;
    }

    return false;
}

static inline bool PickTokenOnPos(DescentState* state, const size_t pos, TokenId tokenId)
{
    assert(state);

    if (state->tokens.data[pos].valueType     == TokenValueType::TOKEN &&
        state->tokens.data[pos].value.tokenId == tokenId)
        return true;
    
    return false;
}

static inline TokenId GetLastTokenId(DescentState* state)
{
    assert(state);
    assert(state->tokens.data[POS(state)].valueType == TokenValueType::TOKEN);

    return state->tokens.data[POS(state)].value.tokenId;
}

void CodeParse(const char* str, SyntaxParserErrors* outErr, FILE* outStream)
{
    assert(str);

    Tree expression = {};

    DescentState state = {};
    DescentStateCtor(&state, str);

    ParseOnTokens(str, &state.tokens);

    bool err = false;
    expression.root = GetGrammar(&state, &err);

    if (err)
        *outErr = SyntaxParserErrors::SYNTAX_ERR; 

    if (!err)
    {
        TreeGraphicDump(&expression, true, state.allNamesTable);
        TreePrintPrefixFormat(&expression, outStream, state.allNamesTable);
    }

    DescentStateDtor(&state);
}

static TreeNode* GetGrammar(DescentState* state, bool* outErr)
{
    assert(state);
    assert(outErr);

    TreeNode* root = GetFunc(state, outErr);
    IF_ERR_RET(outErr, root, nullptr);

    while (!PickToken(state, TokenId::PROGRAM_END))
    {
        TreeNode* tmpNode = GetFunc(state, outErr);
        IF_ERR_RET(outErr, root, tmpNode);

        root = MAKE_NEW_FUNC_NODE(root, tmpNode);
    }

    SynAssert(state, PickToken(state, TokenId::PROGRAM_END), outErr);
    IF_ERR_RET(outErr, root, nullptr);

    return root;
}

static TreeNode* GetFunc(DescentState* state, bool* outErr)
{
    TreeNode* node = GetFuncDef(state, outErr);
    IF_ERR_RET(outErr, node, nullptr);

    return node;
}

static TreeNode* GetFuncDef(DescentState* state, bool* outErr)
{
    NameTableType* localNameTable = nullptr;
    NameTableCtor(&localNameTable);

    TreeNode* func = nullptr;

    TreeNode* typeNode = GetType(state, outErr);
    IF_ERR_RET(outErr, typeNode, nullptr);

    TreeNode* funcName = AddVar(state, outErr);
    IF_ERR_RET(outErr, typeNode, funcName);
    
    //TODO: create set table function maybe
    state->globalTable->data[funcName->value.nameId].localNameTable = (void*)localNameTable;
    state->currentLocalTable = localNameTable;
    func = MAKE_FUNC_NODE(funcName);

    TreeNode* funcVars = GetFuncVarsDef(state, outErr);
    funcName->left = funcVars;
    IF_ERR_RET(outErr, func, typeNode);

    SynAssert(state, PickToken(state, TokenId::FIFTY_SEVEN), outErr);
    IF_ERR_RET(outErr, func, typeNode);
    
    TreeNode* funcCode = GetOp(state, outErr);
    funcName->right = funcCode;
    IF_ERR_RET(outErr, func, typeNode);

    func = MAKE_TYPE_NODE(typeNode, func);

    return func;
}

static TreeNode* GetType(DescentState* state, bool* outErr)
{
    ConsumeToken(state, TokenId::TYPE_INT, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    return TreeNodeCreate(TreeNodeOpValueCreate(TreeOperationId::TYPE_INT), 
                                                    TreeNodeValueType::OPERATION);
}

static TreeNode* GetFuncVarsDef(DescentState* state, bool* outErr)
{
    TreeNode* varsDefNode = nullptr;
        
    if (!PickToken(state, TokenId::TYPE_INT))
        return nullptr;
    
    TreeNode* varType = GetType(state, outErr);
    IF_ERR_RET(outErr, varType, nullptr);
    
    TreeNode* varName = AddVar(state, outErr);
    IF_ERR_RET(outErr, varType, varName);

    varsDefNode = MAKE_TYPE_NODE(varType, varName);

    while (!PickToken(state, TokenId::FIFTY_SEVEN))
    {
        varType = GetType(state, outErr);
        IF_ERR_RET(outErr, varsDefNode, varType);

        varName = AddVar(state, outErr);
        TreeNode* tmpVar = MAKE_TYPE_NODE(varType, varName);
        IF_ERR_RET(outErr, tmpVar, varsDefNode);

        varsDefNode = MAKE_COMMA_NODE(varsDefNode, tmpVar);
    }

    return varsDefNode;
}

static TreeNode* GetOp(DescentState* state, bool* outErr)
{
    TreeNode* opNode = nullptr;

    if (PickToken(state, TokenId::IF))
    {
        opNode = GetIf(state, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        return opNode;
    }
    else if (PickToken(state, TokenId::WHILE))
    {
        opNode = GetWhile(state, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        return opNode;
    }
    else if (PickToken(state, TokenId::L_BRACE))
        opNode = GetPrint(state, outErr);
    else if (PickToken(state, TokenId::TYPE_INT))
        opNode = GetVarDef(state, outErr);
    else if (PickTokenOnPos(state, state->tokenPos + 1, TokenId::ASSIGN))
        opNode = GetAssign(state, outErr);
    else if (PickToken(state, TokenId::FIFTY_SEVEN))
    {
        ConsumeToken(state, TokenId::FIFTY_SEVEN, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        TreeNode* jointNode = MAKE_LINE_END_NODE(nullptr, nullptr);
        opNode = jointNode;
        while (true)
        {
            //printf(RED_TEXT("Going in op recursively\n"));
            TreeNode* opInNode = GetOp(state, outErr);
            IF_ERR_RET(outErr, opNode, opInNode);

            jointNode->left = opInNode;

            if (PickToken(state, TokenId::L_BRACE) && 
                !PickTokenOnPos(state, state->tokenPos + 2, TokenId::FIFTY_SEVEN))
                break;

            jointNode->right = MAKE_LINE_END_NODE(nullptr, nullptr);
            jointNode        = jointNode->right;
        }

        //printf(GREEN_TEXT("OUT OF RECURSION\n"));

        ConsumeToken(state, TokenId::L_BRACE, outErr);
        IF_ERR_RET(outErr, opNode, nullptr);

        return opNode;
    }
    else
        opNode = GetReturn(state, outErr);

    IF_ERR_RET(outErr, opNode, nullptr);

    ConsumeToken(state, TokenId::FIFTY_SEVEN, outErr);
    IF_ERR_RET(outErr, opNode, nullptr);

    return opNode;
}

static TreeNode* GetReturn(DescentState* state, bool* outErr)
{
    return MAKE_RETURN_NODE(GetOr(state, outErr));
}

static TreeNode* GetIf(DescentState* state, bool* outErr)
{
    ConsumeToken(state, TokenId::IF, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    TreeNode* condition = GetOr(state, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    ConsumeToken(state, TokenId::FIFTY_SEVEN, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    TreeNode* op = GetOp(state, outErr);
    IF_ERR_RET(outErr, condition, op);

    return MAKE_IF_NODE(condition, op);
}

static TreeNode* GetWhile(DescentState* state, bool* outErr)
{
    ConsumeToken(state, TokenId::WHILE, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    TreeNode* condition = GetOr(state, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    ConsumeToken(state, TokenId::FIFTY_SEVEN, outErr);
    IF_ERR_RET(outErr, condition, nullptr);

    TreeNode* op = GetOp(state, outErr);
    IF_ERR_RET(outErr, condition, op);

    return MAKE_WHILE_NODE(condition, op);
}

static TreeNode* GetVarDef(DescentState* state, bool* outErr)
{
    TreeNode* typeNode = GetType(state, outErr);
    IF_ERR_RET(outErr, typeNode, nullptr);

    TreeNode* varName = AddVar(state, outErr);
    IF_ERR_RET(outErr, typeNode, varName);

    ConsumeToken(state, TokenId::ASSIGN, outErr);
    IF_ERR_RET(outErr, typeNode, varName);

    TreeNode* expr   = GetOr(state, outErr);
    TreeNode* assign = MAKE_ASSIGN_NODE(varName, expr);
    IF_ERR_RET(outErr, expr, typeNode);

    return MAKE_TYPE_NODE(typeNode, assign);
}

static TreeNode* GetExpr(DescentState* state, bool* outErr)
{
    if (!PickToken(state, TokenId::L_BRACKET))
        return GetArg(state, outErr);

    ConsumeToken(state, TokenId::L_BRACKET, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);
    
    TreeNode* inBracketsExpr = GetOr(state, outErr);
    IF_ERR_RET(outErr, inBracketsExpr, nullptr);

    ConsumeToken(state, TokenId::R_BRACKET, outErr); //Здесь изменение, раньше не было сдвига pos, предполагаю что баг
    IF_ERR_RET(outErr, inBracketsExpr, nullptr);

    return inBracketsExpr;
}

static TreeNode* GetPrint(DescentState* state, bool* outErr)
{
    ConsumeToken(state, TokenId::L_BRACE, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    TreeNode* arg = GetArg(state, outErr);
    IF_ERR_RET(outErr, arg, nullptr);

    return MAKE_PRINT_NODE(arg);
}

static TreeNode* GetRead(DescentState* state, bool* outErr)
{
    ConsumeToken(state, TokenId::L_BRACE, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    return MAKE_READ_NODE(nullptr, nullptr);
}

static TreeNode* GetAssign(DescentState* state, bool* outErr)
{
    TreeNode* var = GetVar(state, outErr);
    IF_ERR_RET(outErr, var, nullptr);

    ConsumeToken(state, TokenId::ASSIGN, outErr);
    IF_ERR_RET(outErr, var, nullptr);

    TreeNode* rightExpr = GetOr(state, outErr);
    IF_ERR_RET(outErr, rightExpr, var);

    return MAKE_ASSIGN_NODE(var, rightExpr);
}

static TreeNode* GetMadeFuncCall(DescentState* state, bool* outErr)
{
    TreeNode* funcName = GetVar(state, outErr);
    IF_ERR_RET(outErr, funcName, nullptr);

    ConsumeToken(state, TokenId::L_BRACE, outErr);
    IF_ERR_RET(outErr, funcName, nullptr);

    TreeNode* funcVars = GetFuncVarsCall(state, outErr);
    IF_ERR_RET(outErr, funcName, funcVars);
    funcName->left = funcVars;

    ConsumeToken(state, TokenId::FIFTY_SEVEN, outErr);
    IF_ERR_RET(outErr, funcName, funcVars);

    return MAKE_FUNC_CALL_NODE(funcName);
}

static TreeNode* GetFuncVarsCall(DescentState* state, bool* outErr)
{
    if (PickToken(state, TokenId::FIFTY_SEVEN))
        return nullptr;

    TreeNode* vars = GetOr(state, outErr);
    IF_ERR_RET(outErr, vars, nullptr);

    while (!PickToken(state, TokenId::FIFTY_SEVEN))
    {
        TreeNode* tmpVar = GetOr(state, outErr);
        IF_ERR_RET(outErr, vars, tmpVar);

        vars = MAKE_COMMA_NODE(vars, tmpVar);
    }
    
    return vars;
}

static TreeNode* GetOr(DescentState* state, bool* outErr)
{
    TreeNode* allExpr = GetAnd(state, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (PickToken(state, TokenId::OR))
    {
        ConsumeToken(state, TokenId::OR, outErr);
        IF_ERR_RET(outErr, allExpr, nullptr);

        TreeNode* tmpExpr = GetAnd(state, outErr);
        IF_ERR_RET(outErr, tmpExpr, allExpr);

        allExpr = MAKE_OR_NODE(allExpr, tmpExpr);
    }

    return allExpr;
}

static TreeNode* GetAnd(DescentState* state, bool* outErr)
{
    TreeNode* allExpr = GetCmp(state, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (PickToken(state, TokenId::AND))
    {
        ConsumeToken(state, TokenId::OR, outErr);
        IF_ERR_RET(outErr, allExpr, nullptr);

        TreeNode* tmpExpr = GetCmp(state, outErr);
        IF_ERR_RET(outErr, tmpExpr, allExpr);

        allExpr = MAKE_AND_NODE(allExpr, tmpExpr);
    }

    return allExpr;
}

static TreeNode* GetCmp(DescentState* state, bool* outErr)
{
    TreeNode* allExpr = GetAddSub(state, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (PickToken(state, TokenId::LESS)    || PickToken(state, TokenId::LESS_EQ)     ||
           PickToken(state, TokenId::GREATER) || PickToken(state, TokenId::GREATER_EQ)  ||
           PickToken(state, TokenId::EQ)      || PickToken(state, TokenId::NOT_EQ))
    {
        TokenId tokenId = GetLastTokenId(state);
        POS(state)++;

        TreeNode* newExpr = GetAddSub(state, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        switch (tokenId)
        {
            case TokenId::LESS:
                allExpr = MAKE_LESS_NODE(allExpr, newExpr);
                break;

            case TokenId::LESS_EQ:
                allExpr = MAKE_LESS_EQ_NODE(allExpr, newExpr);
                break;
            
            case TokenId::GREATER:
                allExpr = MAKE_GREATER_NODE(allExpr, newExpr);
                break;

            case TokenId::GREATER_EQ:
                allExpr = MAKE_GREATER_EQ_NODE(allExpr, newExpr);
                break;

            case TokenId::EQ:
                allExpr = MAKE_EQ_NODE(allExpr, newExpr);
                break;

            case TokenId::NOT_EQ:
                allExpr = MAKE_NOT_EQ_NODE(allExpr, newExpr);
                break;
            default:
                SynAssert(state, false, outErr);
                IF_ERR_RET(outErr, allExpr, newExpr);
                break;
        }
    }

    return allExpr;
}

static TreeNode* GetAddSub(DescentState* state, bool* outErr)
{
    TreeNode* allExpr = GetMulDiv(state, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (PickToken(state, TokenId::ADD) || PickToken(state, TokenId::SUB))
    {
        TokenId tokenId = GetLastTokenId(state);
        POS(state)++;

        TreeNode* newExpr = GetMulDiv(state, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        switch (tokenId)
        {
            case TokenId::ADD:
                allExpr = MAKE_ADD_NODE(allExpr, newExpr);
                break;
            
            case TokenId::SUB:
                allExpr = MAKE_SUB_NODE(allExpr, newExpr);
                break;

            default:
                SynAssert(state, false, outErr);
                IF_ERR_RET(outErr, allExpr, newExpr);
                break;
        }
    }

    return allExpr;
}

static TreeNode* GetMulDiv(DescentState* state, bool* outErr)
{
    TreeNode* allExpr = GetPow(state, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (PickToken(state, TokenId::MUL) || PickToken(state, TokenId::DIV))
    {
        TokenId tokenId = GetLastTokenId(state);
        POS(state)++;

        TreeNode* newExpr = GetPow(state, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);

        switch (tokenId)
        {
            case TokenId::MUL:
                allExpr = MAKE_MUL_NODE(allExpr, newExpr);
                break;
            
            case TokenId::DIV:
                allExpr = MAKE_DIV_NODE(allExpr, newExpr);
                break;

            default:
                SynAssert(state, false, outErr);
                IF_ERR_RET(outErr, allExpr, newExpr);
                break;
        }
    }

    return allExpr;
}

static TreeNode* GetPow(DescentState* state, bool* outErr)
{
    TreeNode* allExpr = GetFuncCall(state, outErr);
    IF_ERR_RET(outErr, allExpr, nullptr);

    while (PickToken(state, TokenId::POW))
    {
        TokenId tokenId = GetLastTokenId(state);
        POS(state)++;

        TreeNode* newExpr = GetFuncCall(state, outErr);
        IF_ERR_RET(outErr, allExpr, newExpr);
        
        assert(tokenId == TokenId::POW);
        allExpr = MAKE_POW_NODE(allExpr, newExpr);
    }

    return allExpr;
}

static TreeNode* GetFuncCall(DescentState* state, bool* outErr)
{
    if (PickToken(state, TokenId::SIN)  || PickToken(state, TokenId::COS) ||
        PickToken(state, TokenId::TAN)  || PickToken(state, TokenId::COT) ||
        PickToken(state, TokenId::SQRT) || PickToken(state, TokenId::L_BRACE))
        return GetBuiltInFuncCall(state, outErr);

    if (PickTokenOnPos(state, state->tokenPos + 1, TokenId::L_BRACE))
        return GetMadeFuncCall(state, outErr);
    
    return GetArg(state, outErr);
}

static TreeNode* GetBuiltInFuncCall(DescentState* state, bool* outErr)
{
    if (PickToken(state, TokenId::L_BRACE))
        return GetRead(state, outErr);

    if (!(PickToken(state, TokenId::SIN) || PickToken(state, TokenId::COS) ||
          PickToken(state, TokenId::TAN) || PickToken(state, TokenId::COT) ||
          PickToken(state, TokenId::SQRT)))
        return GetExpr(state, outErr);

    TokenId tokenId = GetLastTokenId(state);

    ConsumeToken(state, TokenId::L_BRACKET, outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    TreeNode* expr = GetOr(state, outErr);
    IF_ERR_RET(outErr, expr, nullptr);

    ConsumeToken(state, TokenId::R_BRACKET, outErr);
    IF_ERR_RET(outErr, expr, nullptr);

    switch (tokenId)
    {
        case TokenId::SIN:
            expr = MAKE_SIN_NODE(expr);
            break;

        case TokenId::COS:
            expr = MAKE_COS_NODE(expr);
            break;
        
        case TokenId::TAN:
            expr = MAKE_TAN_NODE(expr);
            break;

        case TokenId::COT:
            expr = MAKE_COT_NODE(expr);
            break;

        case TokenId::SQRT:
            expr = MAKE_SQRT_NODE(expr);
            break;

        default:
            SynAssert(state, false, outErr);
            IF_ERR_RET(outErr, expr, nullptr);
            break;
    }

    return expr;
}

static TreeNode* GetArg(DescentState* state, bool* outErr)
{
    TreeNode* arg = nullptr;

    if (PickNum(state))
        arg = GetNum(state, outErr);
    else 
        arg = GetVar(state, outErr);

    IF_ERR_RET(outErr, arg, nullptr);

    return arg;
}

static TreeNode* GetNum(DescentState* state, bool* outErr)
{
    SynAssert(state, PickNum(state), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    TreeNode* num = MAKE_NUM(state->tokens.data[POS(state)].value.num);
    POS(state)++;

    return num;
}

static TreeNode* AddVar(DescentState* state, bool* outErr)
{
    SynAssert(state, PickName(state), outErr);
    IF_ERR_RET(outErr, nullptr, nullptr);

    Name pushName = 
    {
        .name           = strdup(state->tokens.data[POS(state)].value.name),

        .localNameTable = nullptr,
    };

    TreeNode* varNode = nullptr;

    NameTablePush(state->currentLocalTable, pushName);
    NameTablePush(state->allNamesTable, pushName);
    varNode = MAKE_VAR(state->allNamesTable->size - 1);
    
    POS(state)++;

    return varNode;
}

//TODO: сюда передавать localNameTable, тут его сувать(мб)
static TreeNode* GetVar(DescentState* state, bool* outErr)
{
    //TODO: создать отдельную функцию createName
    Name pushName = 
    {
        .name           = strdup(state->tokens.data[POS(state)].value.name),

        .localNameTable = nullptr,
    };

    //TODO: здесь проверки на то, что мы пушим (в плане того, чтобы не было конфликтов имен и т.д, пока похуй)
    TreeNode* varNode = nullptr;

    Name* outName = nullptr;
    NameTableFind(state->allNamesTable, pushName.name, &outName);

    SynAssert(state, outName != nullptr, outErr);
    IF_ERR_RET(outErr, varNode, nullptr);

    //TODO: здесь пройтись по локали + глобали, проверить на существование переменную типо
    varNode = MAKE_VAR(outName - state->allNamesTable->data);

    POS(state)++;

    return varNode;
}


static void DescentStateCtor(DescentState* state, const char* str)
{
    TokensArrCtor(&state->tokens);
    NameTableCtor(&state->globalTable);
    NameTableCtor(&state->allNamesTable);

    state->currentLocalTable = state->globalTable;

    state->codeString       = str;
    state->tokenPos  = 0;
}

static void DescentStateDtor(DescentState* state)
{
    
    for (size_t i = 0; i < state->globalTable->size; ++i)
    {
        if (state->globalTable->data[i].localNameTable)
            NameTableDtor((NameTableType*)state->globalTable->data[i].localNameTable);
    }

    NameTableDtor(state->globalTable);

    TokensArrDtor(&state->tokens);
    state->tokenPos = 0;
}
