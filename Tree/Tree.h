#ifndef TREE_H
#define TREE_H

#include <stdio.h>

#define GENERATE_OPERATION_CMD(NAME, ...) NAME, 

enum class TreeOperationId
{
    #include "Operations.h"
};

#undef GENERATE_OPERATION_CMD

union TreeNodeValue
{
    double                  value;
    char*                   varPtr; //Pointer to the array
    TreeOperationId operation;
}; 

enum class TreeNodeValueTypeof
{
    VALUE,
    VARIABLE,
    OPERATION, 
};

struct TreeNodeType
{
    TreeNodeValue        value;
    TreeNodeValueTypeof  valueType;
    
    TreeNodeType*  left;
    TreeNodeType* right;
};

struct TreeType
{
    TreeNodeType* root;
};

enum class TreeErrors
{
    NO_ERR,

    MEM_ERR,

    READING_ERR,

    CAPACITY_ERR,
    VARIABLE_NAME_ERR, 
    VARIABLE_VAL_ERR,
    VARIABLES_DATA_ERR,

    NODE_EDGES_ERR,

    NO_REPLACEMENT,
};

//-------------Expression main funcs----------

TreeErrors TreeCtor(TreeType* tree);
TreeErrors TreeDtor(TreeType* tree);

TreeNodeType* TreeNodeCreate(TreeNodeValue value, 
                                            TreeNodeValueTypeof valueType,
                                            TreeNodeType* left  = nullptr,
                                            TreeNodeType* right = nullptr);
void TreeNodeDtor(TreeNodeType* node);

TreeErrors TreeVerify     (const TreeType*      tree);
TreeErrors TreeVerify     (const TreeNodeType* node);

TreeNodeValue TreeNodeValueCreate(double value);
TreeNodeValue TreeNodeValueCreate(TreeOperationId operationId);

//Expects pointer to the allocated memory for var name
TreeNodeValue TreeNodeValueCreate(char* varPtr);

TreeNodeType* TreeNumericNodeCreate(double value);
TreeNodeType* TreeVariableNodeCreate(char* varPtr);

#define TREE_TEXT_DUMP(tree) TreeTextDump((tree), __FILE__, \
                                                  __func__, \
                                                  __LINE__)

void TreeTextDump(const TreeType* tree, const char* fileName, 
                                                          const char* funcName,
                                                          const int   line);

void TreeGraphicDump(const TreeType* tree, bool openImg = false);

#define TREE_DUMP(tree) TreeDump((tree), __FILE__,  \
                                                                          __func__,  \
                                                                          __LINE__)

void TreeDump(const TreeType* tree, const char* fileName,
                                                              const char* funcName,
                                                              const int   line);

void TreeNodeSetEdges(TreeNodeType* node, TreeNodeType* left, 
                                                         TreeNodeType* right);

//TreeType       TreeCopy(const TreeType* tree);
//TreeNodeType* TreeNodeCopy(const TreeNodeType* node);

TreeErrors TreePrintPrefixFormat  (const TreeType* tree, 
                                                        FILE* outStream = stdout);
TreeErrors TreeReadPrefixFormat  (TreeType* tree, FILE* inStream = stdin);

//-------------Operations funcs-----------

int  TreeOperationGetId(const char* string);
const char* TreeOperationGetLongName (const  TreeOperationId operation);
const char* TreeOperationGetShortName(const TreeOperationId operation);
bool TreeOperationIsUnary(const TreeOperationId operation);

#endif