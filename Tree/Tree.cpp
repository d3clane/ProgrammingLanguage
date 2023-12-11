#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "Tree.h"
#include "Common/StringFuncs.h"
#include "Common/Log.h"
#include "FastInput/InputOutput.h"
#include "Common/DoubleFuncs.h"

//---------------------------------------------------------------------------------------

static void TreeDtor     (TreeNodeType* node);


static TreeErrors TreePrintPrefixFormat     (
                                                const TreeNodeType* node, 
                                                FILE* outStream);

static TreeNodeType* TreeReadPrefixFormat(const char* const string, 
                                                       const char** stringEndPtr);
static const char* TreeReadTokenValue(TreeNodeValue* value, 
                                               TreeNodeValueTypeof* valueType, 
                                               const char* string);

static void TreeGraphicDump(const TreeNodeType* node, FILE* outDotFile);
static void DotFileCreateTokens(const TreeNodeType* node, FILE* outDotFile);

static inline void CreateImgInLogFile(const size_t imgIndex, bool openImg);
static inline void DotFileBegin(FILE* outDotFile);
static inline void DotFileEnd  (FILE* outDotFile);

#define TREE_CHECK(tree)                        \
do                                                          \
{                                                           \
    TreeErrors err = TreeVerify(tree);    \
                                                            \
    if (err != TreeErrors::NO_ERR)                    \
        return err;                                         \
} while (0)

//---------------------------------------------------------------------------------------

//TODO: докинуть возможность ввода capacity
TreeErrors TreeCtor(TreeType* tree)
{
    assert(tree);

    tree->root = nullptr;

    TREE_CHECK(tree);

    return TreeErrors::NO_ERR;
}

//---------------------------------------------------------------------------------------

TreeErrors TreeDtor(TreeType* tree)
{
    assert(tree);

    TreeDtor(tree->root);
    tree->root = nullptr;

    return TreeErrors::NO_ERR;
}

//---------------------------------------------------------------------------------------

static void TreeDtor(TreeNodeType* node)
{
    if (node == nullptr)
        return;
    
    assert(node);

    TreeDtor(node->left);
    TreeDtor(node->right);

    TreeNodeDtor(node);
}

//---------------------------------------------------------------------------------------

TreeNodeType* TreeNodeCreate(TreeNodeValue value, 
                                            TreeNodeValueTypeof valueType,
                                            TreeNodeType* left,
                                            TreeNodeType* right)
{   
    TreeNodeType* node = (TreeNodeType*)calloc(1, sizeof(*node));
    node->left      = left;
    node->right     = right;
    node->value     = value;
    node->valueType = valueType;
    
    return node;
}

//---------------------------------------------------------------------------------------

void TreeNodeDtor(TreeNodeType* node)
{
    node->left         = nullptr;
    node->right        = nullptr;
    node->value.varPtr = nullptr;

    free(node);
}

//---------------------------------------------------------------------------------------

TreeErrors TreeVerify(const TreeType* tree)
{
    assert(tree);

    return TreeVerify(tree->root);
}

TreeErrors TreeVerify(const TreeNodeType* node)
{
    if (node == nullptr)
        return TreeErrors::NO_ERR;

    TreeErrors err = TreeVerify(node->left);
    if (err != TreeErrors::NO_ERR) return err;

    err = TreeVerify(node->right);
    if (err != TreeErrors::NO_ERR) return err;

    if (node->left == node->right && node->left != nullptr)
        return TreeErrors::NODE_EDGES_ERR;
    
    if (node->left == node || node->right == node)
        return TreeErrors::NODE_EDGES_ERR;

    return TreeErrors::NO_ERR;
}

//---------------------------------------------------------------------------------------

static inline void CreateImgInLogFile(const size_t imgIndex, bool openImg)
{
    static const size_t maxImgNameLength  = 64;
    static char imgName[maxImgNameLength] = "";
    snprintf(imgName, maxImgNameLength, "../imgs/img_%zu_time_%s.png", imgIndex, __TIME__);

    static const size_t     maxCommandLength  = 128;
    static char commandName[maxCommandLength] =  "";
    snprintf(commandName, maxCommandLength, "dot TreeHandler.dot -T png -o %s", imgName);
    system(commandName);

    snprintf(commandName, maxCommandLength, "<img src = \"%s\">\n", imgName);    
    Log(commandName);

    if (openImg)
    {
        snprintf(commandName, maxCommandLength, "open %s", imgName);
        system(commandName);
    }
}

//---------------------------------------------------------------------------------------

static inline void DotFileBegin(FILE* outDotFile)
{
    fprintf(outDotFile, "digraph G{\nrankdir=TB;\ngraph [bgcolor=\"#31353b\"];\n"
                        "edge[color=\"#00D0D0\"];\n");
}

//---------------------------------------------------------------------------------------

static inline void DotFileEnd(FILE* outDotFile)
{
    fprintf(outDotFile, "\n}\n");
}

//---------------------------------------------------------------------------------------

void TreeGraphicDump(const TreeType* tree, bool openImg)
{
    assert(tree);

    static const char* dotFileName = "treeHandler.dot";
    FILE* outDotFile = fopen(dotFileName, "w");

    if (outDotFile == nullptr)
        return;

    DotFileBegin(outDotFile);

    DotFileCreateTokens(tree->root, outDotFile);

    TreeGraphicDump(tree->root, outDotFile);

    DotFileEnd(outDotFile);

    fclose(outDotFile);

    static size_t imgIndex = 0;
    CreateImgInLogFile(imgIndex, openImg);
    imgIndex++;
}

//---------------------------------------------------------------------------------------

static void DotFileCreateTokens(const TreeNodeType* node, FILE* outDotFile)
{
    assert(outDotFile);

    if (node == nullptr)
        return;
    
    fprintf(outDotFile, "node%p"
                        "[shape=Mrecord, style=filled, ", node);

    if (node->valueType == TreeNodeValueTypeof::OPERATION)
        fprintf(outDotFile, "fillcolor=\"#89AC76\", label = \"%s\", ", 
                            TreeOperationGetLongName(node->value.operation));
    else if (node->valueType == TreeNodeValueTypeof::VALUE)
        fprintf(outDotFile, "fillcolor=\"#7293ba\", label = \"%lg\", ", node->value.value);
    else if (node->valueType == TreeNodeValueTypeof::VARIABLE)
        fprintf(outDotFile, "fillcolor=\"#78DBE2\", label = \"%s\", ",
                            node->value.varPtr);
    else 
        fprintf(outDotFile, "fillcolor=\"#FF0000\", label = \"ERROR\", ");

    fprintf(outDotFile, "color = \"#D0D000\"];\n");

    DotFileCreateTokens(node->left, outDotFile);
    DotFileCreateTokens(node->right, outDotFile);
}

//---------------------------------------------------------------------------------------

static void TreeGraphicDump(const TreeNodeType* node, FILE* outDotFile)
{
    if (node == nullptr)
    {
        fprintf(outDotFile, "\n");
        return;
    }
    
    fprintf(outDotFile, "node%p;\n", node);

    if (node->left != nullptr) fprintf(outDotFile, "node%p->", node);
    TreeGraphicDump(node->left, outDotFile);

    if (node->right != nullptr) fprintf(outDotFile, "node%p->", node);
    TreeGraphicDump(node->right, outDotFile);
}

//---------------------------------------------------------------------------------------

void TreeTextDump(const TreeType* tree, const char* fileName, 
                                                          const char* funcName,
                                                          const int   line)
{
    assert(tree);
    assert(fileName);
    assert(funcName);

    LogBegin(fileName, funcName, line);

    Log("Tree root: %p, value: %s\n", tree->root, tree->root->value);
    Log("Tree: ");
    TreePrintPrefixFormat(tree, nullptr);

    LOG_END();
}

//---------------------------------------------------------------------------------------

void TreeDump(const TreeType* tree, const char* fileName,
                                                              const char* funcName,
                                                              const int   line)
{
    assert(tree);
    assert(fileName);
    assert(funcName);

    TreeTextDump(tree, fileName, funcName, line);

    TreeGraphicDump(tree);
}

//---------------------------------------------------------------------------------------

/*
TreeType TreeCopy(const TreeType* tree)
{
    TreeType copyExpr = {};
    TreeCtor(&copyExpr);

    TreeNodeType* copyExprRoot = TreeNodeCopy(tree->root);

    copyExpr.root = copyExprRoot;

    return copyExpr;
}

TreeNodeType* TreeNodeCopy(const TreeNodeType* node)
{
    if (node == nullptr)
        return nullptr;

    TreeNodeType* left  = TreeNodeCopy(node->left);
    TreeNodeType* right = TreeNodeCopy(node->right);

    return TreeNodeCreate(node->value, node->valueType, left, right);
}
*/

//---------------------------------------------------------------------------------------

TreeNodeValue TreeNodeValueCreate(double value)
{
    TreeNodeValue tokenValue =
    {
        .value = value
    };

    return tokenValue;
}

TreeNodeValue TreeNodeValueCreate(TreeOperationId operation)
{
    TreeNodeValue value =
    {
        .operation = operation,
    };

    return value;
}

TreeNodeValue TreeNodeValueCreate(char* varPtr)
{
    TreeNodeValue value = 
    {
        .varPtr = varPtr,
    };

    return value;
}

//---------------------------------------------------------------------------------------

TreeNodeType* TreeNumericNodeCreate(double value)
{
    TreeNodeValue tokenVal = TreeNodeValueCreate(value);

    return TreeNodeCreate(tokenVal, TreeNodeValueTypeof::VALUE);
}

TreeNodeType* TreeVariableNodeCreate(char* varPtr)
{
    assert(varPtr);

    TreeNodeValue tokenVal  = TreeNodeValueCreate(varPtr);

    return TreeNodeCreate(tokenVal, TreeNodeValueTypeof::VARIABLE);
}

//---------------------------------------------------------------------------------------

#define PRINT(outStream, ...)                          \
do                                                     \
{                                                      \
    if (outStream) fprintf(outStream, __VA_ARGS__);    \
    Log(__VA_ARGS__);                                  \
} while (0)

TreeErrors TreePrintPrefixFormat(const TreeType* tree, 
                                             FILE* outStream)
{
    assert(tree);
    assert(outStream);

    LOG_BEGIN();

    TreeErrors err = TreePrintPrefixFormat(tree->root, outStream);

    PRINT(outStream, "\n");

    LOG_END();

    return err;
}

//---------------------------------------------------------------------------------------

static TreeErrors TreePrintPrefixFormat(
                                                    const TreeNodeType* node, 
                                                    FILE* outStream)
{
    if (node == nullptr)
    {
        PRINT(outStream, "nil ");
        return TreeErrors::NO_ERR;
    }

    PRINT(outStream, "(");
    
    if (node->valueType == TreeNodeValueTypeof::VALUE)
        PRINT(outStream, "%.2lg ", node->value.value);
    else if (node->valueType == TreeNodeValueTypeof::VARIABLE)
        PRINT(outStream, "%s ", node->value.varPtr);
    else
        PRINT(outStream, "%s ", TreeOperationGetLongName(node->value.operation));

    TreeErrors err = TreeErrors::NO_ERR;

    err = TreePrintPrefixFormat(node->left, outStream);
    
    err = TreePrintPrefixFormat(node->right, outStream);

    PRINT(outStream, ")");
    
    return err;
}

TreeErrors TreeReadPrefixFormat(TreeType* tree, FILE* inStream)
{
    assert(tree);
    assert(inStream);

    char* inputTree = ReadText(inStream);

    if (inputTree == nullptr)
        return TreeErrors::MEM_ERR;

    const char* inputTreeEndPtr = inputTree;

    tree->root = TreeReadPrefixFormat(inputTree, 
                                                      &inputTreeEndPtr);

    free(inputTree);

    return TreeErrors::NO_ERR;
}

//---------------------------------------------------------------------------------------

static TreeNodeType* TreeReadPrefixFormat(
                                                        const char* const string, 
                                                        const char** stringEndPtr)
{
    assert(string);

    const char* stringPtr = string;

    stringPtr = SkipSymbolsWhileStatement(stringPtr, isspace);

    int symbol = *stringPtr;
    stringPtr++;
    if (symbol != '(') //skipping nils
    {
        int shift = 0;
        sscanf(stringPtr, "%*s%n", &shift);
        stringPtr += shift;

        *stringEndPtr = stringPtr;
        return nullptr;
    }

    TreeNodeValue value;
    TreeNodeValueTypeof valueType;

    stringPtr = TreeReadTokenValue(&value, &valueType, stringPtr);
    TreeNodeType* node = TreeNodeCreate(value, valueType);
    
    TreeNodeType* left  = TreeReadPrefixFormat(stringPtr, &stringPtr);

    TreeNodeType* right = nullptr;
    if (!TreeOperationIsUnary(node->value.operation))
        right = TreeReadPrefixFormat(stringPtr, &stringPtr);

    stringPtr = SkipSymbolsUntilStopChar(stringPtr, ')');
    ++stringPtr;

    TreeNodeSetEdges(node, left, right);

    *stringEndPtr = stringPtr;
    return node;
}

static const char* TreeReadTokenValue(TreeNodeValue* value, 
                                               TreeNodeValueTypeof* valueType, 
                                               const char* string)
{
    assert(value);
    assert(string);
    assert(valueType);

    double readenValue = NAN;
    int shift = 0;
    int scanResult = sscanf(string, "%lf%n\n", &readenValue, &shift);

    if (scanResult != 0)
    {
        value->value = readenValue;
        *valueType   = TreeNodeValueTypeof::VALUE;
        return string + shift;
    }

    shift = 0;

    static const size_t      maxInputStringSize  = 128;
    static char  inputString[maxInputStringSize] =  "";

    const char* stringPtr = string;
    sscanf(string, "%s%n", inputString, &shift);

    stringPtr = string + shift;
    assert(isspace(*stringPtr));

    int operationId = TreeOperationGetId(inputString);
    if (operationId != -1)
    {
        *value     = TreeNodeValueCreate((TreeOperationId) operationId);
        *valueType = TreeNodeValueTypeof::OPERATION;
        return stringPtr;
    }

    char* varName = strdup(inputString);

    assert(varName != nullptr);

    value->varPtr = varName;
    *valueType   = TreeNodeValueTypeof::VARIABLE;

    return stringPtr;
}

void TreeNodeSetEdges(TreeNodeType* node, TreeNodeType* left, 
                                                     TreeNodeType* right)
{
    assert(node);

    node->left  = left;
    node->right = right;
}

int TreeOperationGetId(const char* string)
{
    assert(string);

    #define GENERATE_OPERATION_CMD(NAME, v1, SHORT_NAME, ...) SHORT_NAME,

    static const char* shortNamesArr[] = 
    {
        #include "Operations.h"
    };

    #undef  GENERATE_OPERATION_CMD
    #define GENERATE_OPERATION_CMD(NAME, ...) #NAME, 

    static const char* longNamesArr[] = 
    {
        #include "Operations.h"
    };

    #undef GENERATE_OPERATION_CMD

    static const size_t NumberOfOperations = sizeof(shortNamesArr) / sizeof(*shortNamesArr);

    for (size_t i = 0; i < NumberOfOperations; ++i)
    {
        if (strcasecmp(string, shortNamesArr[i]) == 0 || 
            strcasecmp(string, longNamesArr[i])  == 0)
            return (int)i;
    }

    return -1;
}

const char* TreeOperationGetLongName(const  TreeOperationId operation)
{
    #define GENERATE_OPERATION_CMD(NAME, ...)           \
        case TreeOperationId::NAME:               \
            return #NAME;

    switch (operation)
    {
        #include "Operations.h"

        default:
            break;
    }

    #undef GENERATE_OPERATION_CMD

    return nullptr;
}

const char* TreeOperationGetShortName(const TreeOperationId operation)
{
    #define GENERATE_OPERATION_CMD(NAME, v1, SHORT_NAME, ...)           \
        case TreeOperationId::NAME:                               \
            return SHORT_NAME;
        
    switch(operation)
    {
        #include "Operations.h"

        default:
            break;
    }

    #undef GENERATE_OPERATION_CMD

    return nullptr;
}

bool TreeOperationIsUnary(const TreeOperationId operation)
{

    #define GENERATE_OPERATION_CMD(NAME, IS_UNARY, ...)                                 \
        case TreeOperationId::NAME:                                               \
            return IS_UNARY;
        
    switch (operation)
    {
        #include "Operations.h"

        default:
            break;
    }

    #undef GENERATE_OPERATION_CMD

    return false;
}
