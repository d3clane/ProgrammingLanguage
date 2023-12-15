#include "DSL.h"

#include "Tree.h"


#define GENERATE_OPERATION_CMD(NAME, ...)                                       \
    TreeNodeType* _##NAME(TreeNodeType* left,                                   \
                          TreeNodeType* right)                                  \
    {                                                                           \
        return TreeNodeCreate(TreeNodeOpValueCreate(TreeOperationId::NAME),     \
                              TreeNodeValueTypeof::OPERATION,                   \
                              left, right);                                     \
    }

#include "Operations.h"

#undef GENERATE_OPERATION_CMD
