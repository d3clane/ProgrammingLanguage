#ifndef NAME_TABLE_TYPES_H
#define NAME_TABLE_TYPES_H

/// @file 
/// @brief Contains types for stack

#include <assert.h>
#include <math.h>
#include <stdio.h>

struct Name
{
    char* name;

    void* localNameTable;
    size_t varRamId;
};

/// @brief Chosen NAME_TABLE_POISON value for stack
static const Name NAME_TABLE_POISON = {};

//typedef ElemType (*CopyFuncType)(const ElemType* elem);
//static const CopyFuncType CpyFunc = TokenCopy;

/*
/// @brief Function for checking if two ElemType values are equal 
/// @param [in]a first value
/// @param [in]b second value
/// @return true if they are equal otherwise false
static inline bool Equal(const ElemType* const a, const ElemType* const b)
{
    assert(a);
    assert(b);

    return a == b;
}

static inline bool IsValidValue(const ElemType* value)
{
    return !Equal(value, &NAME_TABLE_POISON);
}
/*static inline bool Equal(const ElemType* const a, const ElemType* const b)
{
    assert(a);
    assert(b);

    int floatClassA = fpclassify(*a);
    int floatClassB = fpclassify(*b);

    if (floatClassA != FP_NORMAL)
    {
        if (floatClassA == floatClassB)
            return true;
        return false;
    }
    
    static const ElemType EPS = 1e-7;

    return fabs(*a - *b) < EPS;
}

static inline bool IsValidValue(const ElemType* value)
{
    return isfinite(*value);
}*/


#endif // TYPES_H