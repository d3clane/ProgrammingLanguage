#ifndef DSL_H
#define DSL_H

#include "Tree.h"

#define MAKE_NUM(VALUE)    TreeNumericNodeCreate(VALUE)
#define MAKE_VAR(NAME)     TreeVariableNodeCreate(NAME)

#define GENERATE_OPERATION_CMD(NAME, ...)                                               \
    TreeNode* MAKE_##NAME ##_NODE(TreeNode* left, TreeNode* right = nullptr);  \

#include "Operations.h"

#undef GENERATE_OPERATION_CMD

#endif