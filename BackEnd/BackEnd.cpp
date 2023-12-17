#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "Tree/NameTable/NameTable.h"
#include "BackEnd.h"

static void AsmCodeBuild(TreeNode* node, NameTableType* localTable, 
                         const NameTableType* allNamesTable, FILE* outStream);
static void AsmCodeBuildFunc(TreeNode* node, const NameTableType* allNamesTable, 
                             size_t* varRamId, FILE* outStream);
static void NameTablePushFuncParams(TreeNode* node, NameTableType* local, 
                                    const NameTableType* allNamesTable, size_t* varRamId);
static void AsmCodeBuildIf(TreeNode* node, NameTableType* localTable, 
                             const NameTableType* allNamesTable, size_t* labelId, FILE* outStream);
static void AsmCodeBuildWhile(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream);
static void AsmCodeBuildAssign(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* varRamId, FILE* outStream);
static void AsmCodeBuildAnd(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream);
static void AsmCodeBuildOr(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream);
static void AsmCodeBuildFuncCall(TreeNode* node, NameTableType* localTable, 
                                 const NameTableType* allNamesTable, FILE* outStream);
static void AsmCodeBuildEq(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream);

void AsmCodeBuild(Tree* tree, NameTableType* allNamesTable, FILE* outStream)
{
    assert(tree);
    assert(allNamesTable);
    assert(outStream);

    fprintf(outStream, "jmp main:\n\n");

    AsmCodeBuild(tree->root, nullptr, allNamesTable, outStream);

    fprintf(outStream, "hlt\n");
}

static void AsmCodeBuild(TreeNode* node, NameTableType* localTable, 
                         const NameTableType* allNamesTable, FILE* outStream)
{
    static size_t varRamId = 0;
    static size_t labelId  = 0;

    if (node == nullptr)
        return;

    if (node->valueType == TreeNodeValueType::NUM)
    {
        fprintf(outStream, "push %d\n", node->value.num);
        return;
    }

    if (node->valueType == TreeNodeValueType::NAME)
    {
        assert(!node->left && !node->right);

        Name* varName = nullptr;
        NameTableFind(localTable, allNamesTable->data[node->value.varId].name, &varName);

        assert(varName);

        fprintf(outStream, "push [%zu]\n", varName->varRamId);
        return;
    }

    assert(node->valueType == TreeNodeValueType::OPERATION);

    switch (node->value.operation)
    {
        case TreeOperationId::ADD:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "add\n");
            break;
        }
        case TreeOperationId::SUB:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "sub\n");
            break;
        }
        case TreeOperationId::MUL:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "mul\n");
            break;
        }
        case TreeOperationId::DIV:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "div\n");
            break;
        }
        case TreeOperationId::SIN:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "sin\n");
            break;
        }
        case TreeOperationId::COS:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "cos\n");
            break;
        }
        case TreeOperationId::TAN:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "tan\n");
            break;
        }
        case TreeOperationId::COT:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "cot\n");
            break;
        }
        case TreeOperationId::SQRT:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            fprintf(outStream, "sqrt\n");
            break;
        }

        case TreeOperationId::COMMA:
        {
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);
            AsmCodeBuild(node->left,  localTable, allNamesTable, outStream);
            // Comma goes in the opposite way, because it is used here only for func call
            break;
        }
        case TreeOperationId::NEW_FUNC:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);

            fprintf(outStream, "\n");

            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            break;
        }

        case TreeOperationId::TYPE:
        {
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);
            break;
        }

        case TreeOperationId::FUNC:
        {
            AsmCodeBuildFunc(node, allNamesTable, &varRamId, outStream);
            
            break;
        }

        case TreeOperationId::LINE_END:
        {
            AsmCodeBuild(node->left, localTable,  allNamesTable, outStream);
            AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

            break;
        }

        case TreeOperationId::IF:
        {
            AsmCodeBuildIf(node, localTable, allNamesTable, &labelId, outStream);
            break;
        }

        case TreeOperationId::WHILE:
        {
            AsmCodeBuildWhile(node, localTable, allNamesTable, &labelId, outStream);
            break;
        }

        case TreeOperationId::AND:
        {
            AsmCodeBuildAnd(node, localTable, allNamesTable, &labelId, outStream);
            break;
        }

        case TreeOperationId::OR:
        {
            AsmCodeBuildOr(node, localTable, allNamesTable, &labelId, outStream);
            break;
        }

        case TreeOperationId::ASSIGN:
        {
            AsmCodeBuildAssign(node, localTable, allNamesTable, &varRamId, outStream);
            break;
        }

        case TreeOperationId::FUNC_CALL:
        {
            AsmCodeBuildFuncCall(node, localTable, allNamesTable, outStream);
            break;
        }

        case TreeOperationId::RETURN:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);

            fprintf(outStream, "ret\n");
            break;
        }

        case TreeOperationId::EQ:
        {
            AsmCodeBuildEq(node, localTable, allNamesTable, &labelId, outStream);
            break;
        }

        case TreeOperationId::READ:
        {
            fprintf(outStream, "in\n");
            break;
        }

        case TreeOperationId::PRINT:
        {
            AsmCodeBuild(node->left, localTable, allNamesTable, outStream);
            fprintf(outStream, "out\n"
                               "pop\n");
    
            break;
        }

        default:
            assert(false);
            break;
    }
}

static void AsmCodeBuildFunc(TreeNode* node, const NameTableType* allNamesTable, 
                             size_t* varRamId, FILE* outStream)
{
    assert(node->left->valueType == TreeNodeValueType::NAME);

    fprintf(outStream, "%s: \n", allNamesTable->data[node->left->value.varId].name);

    NameTableType* local = nullptr;
    NameTableCtor(&local);
    allNamesTable->data[node->left->value.varId].localNameTable = local;

    NameTablePushFuncParams(node->left->left, local, allNamesTable, varRamId);

    for (size_t i = 0; i < local->size; ++i)
        fprintf(outStream, "pop [%zu]\n", local->data[i].varRamId);

    AsmCodeBuild(node->left->right, local, allNamesTable, outStream);
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
        Name pushName =
        {
            .name = strdup(allNamesTable->data[node->value.varId].name),
            
            .localNameTable = nullptr,

            .varRamId       = *varRamId,
        };

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
            assert(false); //TODO: 
            break;
        }
    }
}

static void AsmCodeBuildIf(TreeNode* node, NameTableType* localTable, 
                             const NameTableType* allNamesTable, size_t* labelId, FILE* outStream)
{
    AsmCodeBuild(node->left, localTable, allNamesTable, outStream);

    //TODO: коммент в асме о том что это if
    size_t id = *labelId;
    fprintf(outStream, "\npush 0\n"
                       "je end_if_%zu:\n", id);
    *labelId += 1;

    AsmCodeBuild(node->right, localTable, allNamesTable, outStream);
    fprintf(outStream, "end_if_%zu:\n\n", id);
}

static void AsmCodeBuildWhile(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream)
{
    size_t id = *labelId;
    fprintf(outStream, "while_%zu:\n", id);

    *labelId += 1;
    AsmCodeBuild(node->left, localTable, allNamesTable, outStream);

    fprintf(outStream, "\npush 0\n"
                       "je end_while_%zu:\n", id);
    *labelId += 1;

    AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

    fprintf(outStream, "end_while_%zu:\n\n", id);
}

static void AsmCodeBuildAssign(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* varRamId, FILE* outStream)
{
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

    assert(node->left->valueType == TreeNodeValueType::NAME);
    Name pushName = 
    {
        .name = strdup(allNamesTable->data[node->left->value.varId].name),

        .localNameTable = nullptr,

        .varRamId = *varRamId,
    };

    *varRamId += 1;

    NameTablePush(localTable, pushName);

    fprintf(outStream, "pop [%zu]\n", pushName.varRamId);
}

static void AsmCodeBuildAnd(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

    fprintf(outStream, "mul\n"
                       "push 0\n"
                       "je AND_FALSE_%zu:\n"
                       "push 1\n"
                       "jmp AND_END_%zu:\n"
                       "AND_FALSE_%zu:\n"
                       "push 0\n"
                       "AND_END_%zu:\n",
                       *labelId, *labelId, *labelId, *labelId);

    *labelId += 1;
}

static void AsmCodeBuildOr(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream);

    size_t id = *labelId;

    fprintf(outStream, "push 0\n"
                       "je OR_FIRST_VAL_SET_ZERO_%zu:\n"
                       "push 1\n"
                       "jmp OR_FIRST_VAL_END_%zu:\n"
                       "OR_FIRST_VAL_SET_ZERO_%zu:\n"
                       "push 0\n"
                       "OR_FIRST_VAL_END_%zu\n",
                       id, id, id, id);

    *labelId += 1;

    AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

    fprintf(outStream, "push 0\n"
                       "je OR_SECOND_VAL_SET_ZERO_%zu:\n"
                       "push 1\n"
                       "jmp OR_SECOND_VAL_END_%zu:\n"
                       "OR_SECOND_VAL_SET_ZERO_%zu:\n"
                       "push 0\n"
                       "OR_SECOND_VAL_END_%zu\n",
                       id, id, id, id);

    fprintf(outStream, "add\n"
                       "push 0\n"
                       "je OR_FALSE_%zu:\n"
                       "push 1\n"
                       "jmp OR_END_%zu:\n"
                       "OR_FALSE_%zu:\n"
                       "push 0\n"
                       "OR_END_%zu:\n",
                       id, id, id, id);
}

static void AsmCodeBuildFuncCall(TreeNode* node, NameTableType* localTable, 
                                 const NameTableType* allNamesTable, FILE* outStream)
{
    AsmCodeBuild(node->left->left, localTable, allNamesTable, outStream);

    assert(node->left->valueType == TreeNodeValueType::NAME);
    fprintf(outStream, "call %s:\n", allNamesTable->data[node->left->value.varId].name);

}

static void AsmCodeBuildEq(TreeNode* node, NameTableType* localTable, 
                              const NameTableType* allNamesTable, size_t* labelId, FILE* outStream)
{
    AsmCodeBuild(node->left,  localTable, allNamesTable, outStream);
    AsmCodeBuild(node->right, localTable, allNamesTable, outStream);

    size_t id = *labelId;
    fprintf(outStream, "je EQ_%zu:\n"
                       "push 0\n"
                       "jmp AFTER_EQ_%zu:\n"
                       "EQ_%zu:\n"
                       "push 1\n"
                       "AFTER_EQ_%zu:\n",
                       id, id, id, id); 
    *labelId += 1;
}
