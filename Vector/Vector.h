#ifndef VECTOR_H
#define VECTOR_H

/// @file
/// @brief Contains functions to work with vector

#include <assert.h>

#include "Common/Errors.h"
#include "Types.h"
#include "HashFuncs.h"

//#define VECTOR_CANARY_PROTECTION
//#define VECTOR_HASH_PROTECTION

#ifdef VECTOR_CANARY_PROTECTION

    #define ON_CANARY(...) __VA_ARGS__
    typedef unsigned long long CanaryType;

#else
    
    #define ON_CANARY(...)

#endif

#ifdef VECTOR_HASH_PROTECTION

    #define ON_HASH(...) __VA_ARGS__

#else

    #define ON_HASH(...) 

#endif

///@brief vector dump that substitutes __FILE__, __func__, __LINE__
#define VECTOR_DUMP(STK) VectorDump((STK), __FILE__, __func__, __LINE__)

ON_HASH
(
    ///@brief HashType for hasing function in vector
    typedef HashType HashFuncType(const void* hashingArr, const size_t length, const uint64_t seed);
)

/// @brief Contains all info about data to use it 
struct VectorType
{
    ON_CANARY
    (
        CanaryType structCanaryLeft; ///< left canary for the struct
    )

    ElemType* data;       ///< data with values. Have to be a dynamic array.
    size_t size;          ///< pos to push/pop values (actual size of the data at this moment).
    
    ON_HASH
    (
        
        HashType dataHash;      ///< hash of all elements in data.

        HashFuncType* HashFunc; ///< hashing function

        HashType structHash;    ///< hash of all elements in struct.
    )

    size_t capacity;     ///< REAL size of the data at this moment (calloced more than need at this moment).

    ON_CANARY
    (
        CanaryType structCanaryRight; ///< right canary for the struct
    )
};

/// @brief Errors that can occure while vector is working. 
enum class VectorErrors
{
    VECTOR_NO_ERR,

    VECTOR_MEMORY_ALLOCATION_ERROR,
    VECTOR_EMPTY_ERR, 
    VECTOR_IS_NULLPTR,
    VECTOR_CAPACITY_OUT_OF_RANGE,
    VECTOR_SIZE_OUT_OF_RANGE,
    VECTOR_INVALID_CANARY, 
    VECTOR_INVALID_DATA_HASH,
    VECTOR_INVALID_STRUCT_HASH,
};

#ifdef VECTOR_HASH_PROTECTION
    /// @brief Constructor
    /// @param [out]stk vector to fill
    /// @param [in]capacity size to reserve for the vector
    /// @param [in]HashFunc hash function for calculating hash
    /// @return errors that occurred
    VectorErrors VectorCtor(VectorType* const stk, const size_t capacity = 0, 
                        const HashFuncType HashFunc = MurmurHash);
#else
    /// @brief Constructor
    /// @param [out]stk vector to fill
    /// @param [in]capacity size to reserve for the vector
    /// @return errors that occurred
    VectorErrors VectorCtor(VectorType* const stk, const size_t capacity = 0);
#endif

/// @brief Destructor
/// @param [out]stk vector to destruct
/// @return errors that occurred
VectorErrors VectorDtor(VectorType* const stk);

/// @brief Pushing value to the vector
/// @param [out]stk vector to push in
/// @param [in]val  value to push
/// @return errors that occurred
VectorErrors VectorPush(VectorType* stk, const ElemType val);

/// @brief Popping value to the vector
/// @param [out]stk vector to pop
/// @param [out]retVal popped value
/// @return errors that occurred
VectorErrors VectorPop(VectorType* stk, ElemType* retVal = nullptr);

/// @brief Verifies if vector is used right
/// @param [in]stk vector to verify
/// @return VectorErrors in vector
VectorErrors VectorVerify(VectorType* stk);

/// @brief Prints vector to log-file 
/// @param [in]stk vector to print out
/// @param [in]fileName __FILE__
/// @param [in]funcName __func__
/// @param [in]lineNumber __LINE__
void VectorDump(const VectorType* stk, const char* const fileName, 
                                     const char* const funcName,
                                     const int lineNumber);

/// @brief Checks if vector is empty
/// @param [in]stk vector to check 
/// @return true if vector is empty otherwise false
static inline bool VectorIsEmpty(const VectorType* stk)
{
    assert(stk);
    assert(stk->data);
    
    return stk->size == 0;
}

/// @brief Prints vector error to log file
/// @param [in]error error to print
void VectorPrintError(VectorErrors error);

#endif // VECTOR_H