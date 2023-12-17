#ifndef DSL_H
#define DSL_H

#include "Tree.h"

#define  OP_TYPE_CNST TreeNodeValueTypeof::OPERATION
#define VAR_TYPE_CNST TreeNodeValueTypeof::VARIABLE
#define VAL_TYPE_CNST TreeNodeValueTypeof::VALUE
#define TOKEN_OP(node) node->value.operation

#define VAL_TYPE(node) node->valueType
#define VAL(node)      node->value.value
#define VAR(node)      node->value.varPtr
#define  OP(node)      node->value.operation
#define   L(node)      node->left
#define   R(node)      node->right

#define L_VAL(node) node->left->value.value
#define R_VAL(node) node->right->value.value
#define L_VAR(node) node->left->value.varPtr
#define R_VAR(node) node->right->value.varPtr
#define  L_OP(node) node->left->value.operation
#define  R_OP(node) node->right->value.operation

#define   IS_VAL(node) (node->valueType        == VAL_TYPE_CNST)
#define L_IS_VAL(node) (node->left->valueType  == VAL_TYPE_CNST)
#define R_IS_VAL(node) (node->right->valueType == VAL_TYPE_CNST)
#define   IS_VAR(node) (node->valueType        == VAR_TYPE_CNST)
#define L_IS_VAR(node) (node->left->valueType  == VAR_TYPE_CNST)
#define R_IS_VAR(node) (node->right->valueType == VAR_TYPE_CNST)
#define    IS_OP(node) (node->valueType        ==  OP_TYPE_CNST)
#define  L_IS_OP(node) (node->left->valueType  ==  OP_TYPE_CNST)
#define  R_IS_OP(node) (node->right->valueType ==  OP_TYPE_CNST)

#define CRT_NUM(VALUE)    TreeNumericNodeCreate(VALUE)
#define CRT_VAR(VAR_NAME) TreeVariableNodeCreate(VAR_NAME)

#define GENERATE_OPERATION_CMD(NAME, ...)                                               \
    TreeNode* MAKE_##NAME ##_NODE(TreeNode* left, TreeNode* right = nullptr);  \

#include "Operations.h"

#undef GENERATE_OPERATION_CMD

#endif