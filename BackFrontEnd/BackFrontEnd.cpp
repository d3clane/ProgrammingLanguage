#include <assert.h>

#include "Tree/DSL.h"
#include "BackFrontEnd.h"

static void CodeBuild(TreeNode* node, NameTableType* allNamesTable, FILE* outStream, 
                                                                    size_t numberOfTabs);
static void PrintTabs(size_t numberOfTabs, FILE* outStream);

void CodeBuild(Tree* tree, NameTableType* allNamesTable, FILE* outStream)
{
    assert(tree);
    assert(allNamesTable);
    assert(outStream);

    CodeBuild(tree->root, allNamesTable, outStream, 0);
}

static void CodeBuild(TreeNode* node, NameTableType* allNamesTable, FILE* outStream, 
                                                                    size_t numberOfTabs)
{
    if (node == nullptr)
        return;
    
    if (node->valueType == TreeNodeValueType::NUM)
    {
        fprintf(outStream, "%d", node->value.num);
        return;
    }

    if (node->valueType == TreeNodeValueType::NAME)
    {
        assert(!node->left && !node->right);

        fprintf(outStream, "%s ", allNamesTable->data[node->value.nameId].name);
        return;
    }

    if (node->valueType == TreeNodeValueType::STRING_LITERAL)
    {
        fprintf(outStream, "%s", allNamesTable->data[node->value.nameId].name);
        return;
    }

    assert(node->valueType == TreeNodeValueType::OPERATION);

    switch (node->value.operation)
    {
        case TreeOperationId::NEW_FUNC:
        {
            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, "\n");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            break;
        }

        case TreeOperationId::TYPE_INT:
        {
            fprintf(outStream, "575757 ");
            break;
        }

        case TreeOperationId::FUNC:
        {
            fprintf(outStream, "%s", allNamesTable->data[node->left->value.nameId].name);

            assert(numberOfTabs == 0);

            CodeBuild(node->left->left, allNamesTable, outStream, numberOfTabs + 1);

            fprintf(outStream, "\n57\n");

            CodeBuild(node->left->right, allNamesTable, outStream, numberOfTabs + 1);

            fprintf(outStream, "{\n");

            break;
        }

        case TreeOperationId::LINE_END:
        {
            PrintTabs(numberOfTabs, outStream);

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            if (!(node->left->valueType == TreeNodeValueType::OPERATION && 
                  (node->left->value.operation == TreeOperationId::IF   ||
                   node->left->value.operation == TreeOperationId::WHILE)))
                fprintf(outStream, " 57\n");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            break;
        }

        case TreeOperationId::ASSIGN:
        {
            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " == ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            break;
        }

        case TreeOperationId::READ:
        {
            fprintf(outStream, "{ ");
            
            break;
        }

        case TreeOperationId::PRINT:
        {
            fprintf(outStream, "{ ");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            break;
        }

        case TreeOperationId::FUNC_CALL:
        {
            fprintf(outStream, "%s { ", allNamesTable->data[node->left->value.nameId].name);
            
            CodeBuild(node->left->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " 57");

            break;
        }

        case TreeOperationId::IF:
        {
            fprintf(outStream, "57? ");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " 57\n");
            PrintTabs(numberOfTabs, outStream);
            fprintf(outStream, "57\n");
             
            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs + 1);

            PrintTabs(numberOfTabs, outStream);
            fprintf(outStream, "{\n");

            break;
        }

        case TreeOperationId::WHILE:
        {
            fprintf(outStream, "57! ");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " 57\n");
            PrintTabs(numberOfTabs, outStream);
            fprintf(outStream, "57\n");
             
            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs + 1);

            PrintTabs(numberOfTabs, outStream);
            fprintf(outStream, "{\n");

            break;
        }

        case TreeOperationId::ADD:
        {
            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " - ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::SUB:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " + ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::MUL:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " / ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::DIV:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " * ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::GREATER:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, "< ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::GREATER_EQ:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, "<= ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::LESS:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, "> ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::LESS_EQ:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ">= ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::EQ:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, "!= ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::NOT_EQ:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, "= ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::AND:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " or ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::OR:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, " and ");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::POW:
        {
            fprintf(outStream, "(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ^ (");

            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }

        case TreeOperationId::SIN:
        {
            fprintf(outStream, "sin(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::COS:
        {
            fprintf(outStream, "cos(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::TAN:
        {
            fprintf(outStream, "tan(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::COT:
        {
            fprintf(outStream, "cot(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }
        case TreeOperationId::SQRT:
        {
            fprintf(outStream, "sqrt(");

            CodeBuild(node->left, allNamesTable, outStream, numberOfTabs);

            fprintf(outStream, ") ");

            break;
        }

        default:
        {
            CodeBuild(node->left,  allNamesTable, outStream, numberOfTabs);
            CodeBuild(node->right, allNamesTable, outStream, numberOfTabs);
            break;
        }
    }
}

static void PrintTabs(size_t numberOfTabs, FILE* outStream)
{
    for (size_t i = 0; i < numberOfTabs; ++i)
        fprintf(outStream, "    ");
}
