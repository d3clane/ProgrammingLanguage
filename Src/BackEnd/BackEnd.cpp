#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "Tree/NameTable/NameTable.h"
#include "BackEnd.h"

static void AsmCodeBuild(TreeNode* node, NameTableType* localTable, 
                         const NameTableType* allNamesTable, FILE* outStream, size_t numberOfTabs);

static void AsmCodeBuildFunc(TreeNode* node, const NameTableType* allNamesTable, 
                             size_t* varRamId, FILE* outStream, size_t numberOfTabs);

static void AsmCodePrintStringLiteral(TreeNode* node, NameTableType* localTable, 
                          const NameTableType* allNamesTable, FILE* outStream, size_t numberOfTabs);   

static void NameTablePushFuncParams(TreeNode* node, NameTableType* local, 
                                    const NameTableType* allNamesTable, size_t* varRamId);
static void AsmCodeBuildIf(TreeNode* node, NameTableType* localTable, 
                             const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                             size_t numberOfTabs);
static void AsmCodeBuildWhile(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs);
static void AsmCodeBuildAssign(TreeNode* node, NameTableType* localTable, 
                               const NameTableType* allNamesTable, size_t* varRamId, FILE* outStream,
                               size_t numberOfTabs);
static void AsmCodeBuildAnd(TreeNode* node, NameTableType* localTable, 
                            const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                            size_t numberOfTabs);
static void AsmCodeBuildOr(TreeNode* node, NameTableType* localTable, 
                            const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                            size_t numberOfTabs);
static void AsmCodeBuildFuncCall(TreeNode* node, NameTableType* localTable, 
                                 const NameTableType* allNamesTable, FILE* outStream,
                                 size_t numberOfTabs);
static void AsmCodeBuildEq(TreeNode* node, NameTableType* localTable, 
                            const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                            size_t numberOfTabs);
static void AsmCodeBuildNotEq(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs);
static void AsmCodeBuildGreaterEq(TreeNode* node, NameTableType* localTable, 
                                  const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                                  size_t numberOfTabs);
static void AsmCodeBuildLessEq(TreeNode* node, NameTableType* localTable, 
                               const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                               size_t numberOfTabs);
static void AsmCodeBuildLess(TreeNode* node, NameTableType* localTable, 
                             const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                             size_t numberOfTabs);
static void AsmCodeBuildGreater(TreeNode* node, NameTableType* localTable, 
                                const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                                size_t numberOfTabs);

static void AsmCodeBuildPrint(TreeNode* node, NameTableType* localTable, 
                        const NameTableType* allNamesTable, FILE* outStream, size_t numberOfTabs);

static void PrintTabs(const size_t numberOfTabs, FILE* outStream);
static void FprintfLine(FILE* outStream, const size_t numberOfTabs, const char* format, ...);

void AsmCodeBuild(Tree* tree, NameTableType* allNamesTable, FILE* outStream, FILE* outBinStream)
{
    assert(tree);
    assert(allNamesTable);
    assert(outStream);
    assert(outBinStream);

    fprintf(outStream, "call main:\n"
                       "hlt\n\n");
    
    AsmCodeBuild(tree->root, nullptr, allNamesTable, outStream, 0);

    fprintf(outStream, "    ret\n");
}

static void AsmCodeBuild(TreeNode* node, NameTableType* localTable, 
                         const NameTableType* allNamesTable, FILE* outStream, size_t numberOfTabs)
{
    static size_t varRamId = 0;
    static size_t labelId  = 0;

    if (node == nullptr)
        return;

    if (node->valueType == TreeNodeValueType::NUM)
    {
        FprintfLine(outStream, numberOfTabs, "push %d\n", node->value.num);
        return;
    }

    if (node->valueType == TreeNodeValueType::NAME)
    {
        assert(!node->left && !node->right);

        Name* varName = nullptr;
        NameTableFind(localTable, allNamesTable->data[node->value.nameId].name, &varName);
        assert(varName);

        FprintfLine(outStream, numberOfTabs, "push [%zu]\n", varName->varRamId);

        return;
    }

    if (node->valueType == TreeNodeValueType::STRING_LITERAL)
    {
        AsmCodePrintStringLiteral(node, localTable, allNamesTable, outStream, numberOfTabs);

        return;
    }

    assert(node->valueType == TreeNodeValueType::OPERATION);

    switch (node->value.operation)
    {
        case TreeOperationId::ADD:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "add\n");
            break;
        }
        case TreeOperationId::SUB:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "sub\n");
            break;
        }
        case TreeOperationId::MUL:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "mul\n");
            break;
        }
        case TreeOperationId::DIV:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "div\n");
            break;
        }
        case TreeOperationId::SIN:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "sin\n");
            break;
        }
        case TreeOperationId::COS:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "cos\n");
            break;
        }
        case TreeOperationId::TAN:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "tan\n");
            break;
        }
        case TreeOperationId::COT:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "cot\n");
            break;
        }
        case TreeOperationId::SQRT:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "sqrt\n");
            break;
        }

        case TreeOperationId::COMMA:
        {
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);
            AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);

            // Comma goes in the opposite way, because it is used here only for func call
            break;
        }
        case TreeOperationId::NEW_FUNC:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, "\n");

            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

            break;
        }

        case TreeOperationId::TYPE:
        {
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::FUNC:
        {
            AsmCodeBuildFunc(node, allNamesTable, &varRamId, outStream, numberOfTabs);
            
            break;
        }

        case TreeOperationId::LINE_END:
        {
            AsmCodeBuild(node->left, localTable,  allNamesTable, outStream, numberOfTabs);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

            break;
        }

        case TreeOperationId::IF:
        {
            AsmCodeBuildIf(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::WHILE:
        {
            AsmCodeBuildWhile(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::AND:
        {
            AsmCodeBuildAnd(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::OR:
        {
            AsmCodeBuildOr(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::ASSIGN:
        {
            AsmCodeBuildAssign(node, localTable, allNamesTable, &varRamId, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::FUNC_CALL:
        {
            AsmCodeBuildFuncCall(node, localTable, allNamesTable, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::RETURN:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

            FprintfLine(outStream, numberOfTabs, "ret\n");
            break;
        }

        case TreeOperationId::EQ:
        {
            AsmCodeBuildEq(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }
        case TreeOperationId::NOT_EQ:
        {
            AsmCodeBuildNotEq(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }
        case TreeOperationId::LESS:
        {
            AsmCodeBuildLess(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }
        case TreeOperationId::LESS_EQ:
        {
            AsmCodeBuildLessEq(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }
        case TreeOperationId::GREATER:
        {
            AsmCodeBuildGreater(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }
        case TreeOperationId::GREATER_EQ:
        {
            AsmCodeBuildGreaterEq(node, localTable, allNamesTable, &labelId, outStream, numberOfTabs);
            break;
        }

        case TreeOperationId::READ:
        {
            FprintfLine(outStream, numberOfTabs, "in\n");
            break;
        }

        case TreeOperationId::PRINT:
        {
            AsmCodeBuildPrint(node, localTable, allNamesTable, outStream, numberOfTabs);

            break;
        }

        default:
            assert(false);
            break;
    }
}

static void AsmCodeBuildFunc(TreeNode* node, const NameTableType* allNamesTable, 
                             size_t* varRamId, FILE* outStream, size_t numberOfTabs)
{
    assert(node->left->valueType == TreeNodeValueType::NAME);

    FprintfLine(outStream, numberOfTabs, "%s: \n", 
                        allNamesTable->data[node->left->value.nameId].name);
    numberOfTabs += 1;

    NameTableType* local = nullptr;
    NameTableCtor(&local);

    allNamesTable->data[node->left->value.nameId].localNameTable = local;

    NameTablePushFuncParams(node->left->left, local, allNamesTable, varRamId);

    for (size_t i = 0; i < local->size; ++i)
    {
        FprintfLine(outStream, numberOfTabs, "pop [%zu]\n", local->data[i].varRamId);
    }

    AsmCodeBuild(node->left->right, local, allNamesTable, outStream, numberOfTabs);
}

static void NameTablePushFuncParams(TreeNode* node, NameTableType* local, 
                                    const NameTableType* allNamesTable, size_t* varRamId)
{
    assert(local);
    assert(allNamesTable);

    if (node == nullptr)
        return;

    if (node->valueType == TreeNodeValueType::NAME)
    {
        Name pushName = {};
        NameCtor(&pushName, allNamesTable->data[node->value.nameId].name, nullptr, *varRamId);

        *varRamId += 1;

        NameTablePush(local, pushName);

        return;
    }

    assert(node->valueType == TreeNodeValueType::OPERATION);

    switch (node->value.operation)
    {
        case TreeOperationId::COMMA:
        {
            NameTablePushFuncParams(node->left, local,  allNamesTable, varRamId);
            NameTablePushFuncParams(node->right, local, allNamesTable, varRamId);

            break;
        }
        case TreeOperationId::TYPE:
        {
            NameTablePushFuncParams(node->right, local, allNamesTable, varRamId);
            
            break;
        }   

        default:
        {
            assert(false);
            break;
        }
    }
}

static void AsmCodeBuildIf(TreeNode* node, NameTableType* localTable, 
                           const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                           size_t numberOfTabs)
{
    fprintf(outStream, "\n");
    FprintfLine(outStream, numberOfTabs, "@ if condition:\n");

    AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "je END_IF_%zu:\n", id);
    *labelId += 1;

    FprintfLine(outStream, numberOfTabs, "@ if code block:\n");

    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs + 1);
    
    FprintfLine(outStream, numberOfTabs, "END_IF_%zu:\n\n", id);
}

static void AsmCodeBuildWhile(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs)
{
    size_t id = *labelId;

    fprintf(outStream, "\n");

    FprintfLine(outStream, numberOfTabs, "while_%zu:\n", id);

    *labelId += 1;
    FprintfLine(outStream, numberOfTabs, "@ while condition: \n");

    AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "je END_WHILE_%zu:\n", id);

    *labelId += 1;

    FprintfLine(outStream, numberOfTabs, "@ while code block: \n");

    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs + 1);

    FprintfLine(outStream, numberOfTabs, "END_WHILE_%zu:\n\n", id);
}

static void AsmCodeBuildAssign(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* varRamId, FILE* outStream,
                              size_t numberOfTabs)
{
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    assert(node->left->valueType == TreeNodeValueType::NAME);

    Name* varNameInTablePtr = nullptr;
    NameTableFind(localTable, allNamesTable->data[node->left->value.nameId].name, &varNameInTablePtr);

    //TODO: it is hotfix, a lot of operations are done
    // better fix: split assigning and defining and push only once
    if (varNameInTablePtr == nullptr)
    {
        Name pushName = {};
        NameCtor(&pushName, allNamesTable->data[node->left->value.nameId].name, nullptr, *varRamId);

        *varRamId += 1;

        NameTablePush(localTable, pushName);
        varNameInTablePtr = localTable->data + localTable->size - 1;    // pointing on pushed name
    }

    FprintfLine(outStream, numberOfTabs, "pop [%zu]\n", varNameInTablePtr->varRamId);
}

static void AsmCodeBuildAnd(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    FprintfLine(outStream, numberOfTabs, "mul\n");
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "je AND_FALSE_%zu:\n", *labelId);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "jmp AND_END_%zu:\n", *labelId);
    FprintfLine(outStream, numberOfTabs, "AND_FALSE_%zu:\n", *labelId);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "AND_END_%zu:\n\n", *labelId);

    *labelId += 1;
}

static void AsmCodeBuildOr(TreeNode* node, NameTableType* localTable, 
                           const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                           size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;

    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "je OR_FIRST_VAL_SET_ZERO_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "jmp OR_FIRST_VAL_END_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "OR_FIRST_VAL_SET_ZERO_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "OR_FIRST_VAL_END_%zu\n\n", id);

    *labelId += 1;

    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "je OR_SECOND_VAL_SET_ZERO_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "jmp OR_SECOND_VAL_END_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "OR_SECOND_VAL_SET_ZERO_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "OR_SECOND_VAL_END_%zu\n\n", id);

    FprintfLine(outStream, numberOfTabs, "add\n");
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "je OR_FALSE_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "jmp OR_END_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "OR_FALSE_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "OR_END_%zu:\n\n", id);
}

static void AsmCodeBuildFuncCall(TreeNode* node, NameTableType* localTable, 
                                 const NameTableType* allNamesTable, FILE* outStream,
                                 size_t numberOfTabs)
{
    FprintfLine(outStream, numberOfTabs, "@ pushing func local vars:\n");

    for (size_t i = 0; i < localTable->size; ++i)
    {
        FprintfLine(outStream, numberOfTabs, "push [%zu]\n", localTable->data[i].varRamId);
    }

    FprintfLine(outStream, numberOfTabs, "@ pushing func args:\n");
    AsmCodeBuild(node->left->left, localTable, allNamesTable, outStream, numberOfTabs);

    assert(node->left->valueType == TreeNodeValueType::NAME);

    FprintfLine(outStream, numberOfTabs, 
                    "call %s:\n", allNamesTable->data[node->left->value.nameId].name);

    FprintfLine(outStream, numberOfTabs, "@ saving func return\n");

    FprintfLine(outStream, numberOfTabs, "pop rax\n");

    for (int i = (int)localTable->size - 1; i > -1; --i)
    {
        FprintfLine(outStream, numberOfTabs, "pop [%zu]\n", localTable->data[i].varRamId);
    }

    FprintfLine(outStream, numberOfTabs, "push rax\n");
}

static void AsmCodeBuildEq(TreeNode* node, NameTableType* localTable, 
                           const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                           size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;
    FprintfLine(outStream, numberOfTabs, "je EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "jmp AFTER_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "AFTER_EQ_%zu:\n\n", id);

    *labelId += 1;
}

static void AsmCodeBuildNotEq(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;
    FprintfLine(outStream, numberOfTabs, "jne NOT_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "jmp AFTER_NOT_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "NOT_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "AFTER_NOT_EQ_%zu:\n\n", id);
    
    *labelId += 1;
}

static void AsmCodeBuildLess(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;
    FprintfLine(outStream, numberOfTabs, "jb LESS_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "jmp AFTER_LESS_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "LESS_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "AFTER_LESS_%zu:\n\n", id);
    
    *labelId += 1;
}

static void AsmCodeBuildLessEq(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;
    FprintfLine(outStream, numberOfTabs, "jbe LESS_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "jmp AFTER_LESS_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "LESS_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "AFTER_LESS_EQ_%zu:\n\n", id);
    
    *labelId += 1;
}

static void AsmCodeBuildGreater(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                              size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;
    FprintfLine(outStream, numberOfTabs, "ja GREATER_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "jmp AFTER_GREATER_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "GREATER_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "AFTER_GREATER_%zu:\n\n", id);

    *labelId += 1;
}

static void AsmCodeBuildGreaterEq(TreeNode* node, NameTableType* localTable, 
                                const NameTableType* allNamesTable, size_t* labelId, FILE* outStream,
                                size_t numberOfTabs)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream, numberOfTabs);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream, numberOfTabs);

    size_t id = *labelId;
    FprintfLine(outStream, numberOfTabs, "jae GREATER_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 0\n");
    FprintfLine(outStream, numberOfTabs, "jmp AFTER_GREATER_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "GREATER_EQ_%zu:\n", id);
    FprintfLine(outStream, numberOfTabs, "push 1\n");
    FprintfLine(outStream, numberOfTabs, "AFTER_GREATER_EQ_%zu:\n\n", id);

    *labelId += 1;
}

static void AsmCodePrintStringLiteral(TreeNode* node, NameTableType* localTable, 
                          const NameTableType* allNamesTable, FILE* outStream, size_t numberOfTabs)
{
    const char* string = allNamesTable->data[node->value.nameId].name;

    size_t pos = 1; //skipping " char

    while (string[pos] != '"')
    {
        FprintfLine(outStream, numberOfTabs, "push %d\n", string[pos]);
        FprintfLine(outStream, numberOfTabs, "outc\n");
        FprintfLine(outStream, numberOfTabs, "pop\n");
        ++pos;
    }

    FprintfLine(outStream, numberOfTabs, "push 10\n");
    FprintfLine(outStream, numberOfTabs, "outc\n");
    FprintfLine(outStream, numberOfTabs, "pop\n");
}

static void AsmCodeBuildPrint(TreeNode* node, NameTableType* localTable, 
                        const NameTableType* allNamesTable, FILE* outStream, size_t numberOfTabs)
{
    assert(node->left);

    if (node->left->valueType == TreeNodeValueType::STRING_LITERAL)
    {
        AsmCodePrintStringLiteral(node->left, localTable, allNamesTable, outStream, numberOfTabs);

        return;
    }

    AsmCodeBuild(node->left, localTable, allNamesTable, outStream, numberOfTabs);

    FprintfLine(outStream, numberOfTabs, "out\n");
    FprintfLine(outStream, numberOfTabs, "pop\n");
}

static void PrintTabs(const size_t numberOfTabs, FILE* outStream)
{
    for (size_t i = 0; i < numberOfTabs; ++i)
        fprintf(outStream, "    ");
}

static void FprintfLine(FILE* outStream, const size_t numberOfTabs, const char* format, ...)
{
    va_list args = {};

    va_start(args, format);

    PrintTabs(numberOfTabs, outStream);
    vfprintf(outStream, format, args);

    va_end(args);
}
