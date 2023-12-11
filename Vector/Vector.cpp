#include <math.h>
#include <execinfo.h> 
#include <string.h>

#include "ArrayFuncs.h"
#include "Vector.h"
#include "Common/Log.h"

//----------static functions------------

static VectorErrors VectorRealloc(VectorType* vector, bool increase);

static inline ElemType* MovePtr(ElemType* const data, const size_t moveSz, const int times);

static inline size_t VectorGetSizeForCalloc(VectorType* const vector);

static void VectorDataFill(VectorType* const vector);

static inline bool VectorIsFull(VectorType* vector);

static inline bool VectorIsTooBig(VectorType* vector);

//--------CANARY PROTECTION----------

#ifdef VECTOR_CANARY_PROTECTION

    #define CanaryTypeFormat "%#0llx"
    const CanaryType Canary = 0xDEADBABE;

    const size_t Aligning = 8;

    static inline void CanaryCtor(void* storage)
    {
        assert(storage);
        CanaryType* strg = (CanaryType*) (storage);

        *strg = Canary;
    }

    static inline ElemType* GetAfterFirstCanaryAdr(const VectorType* const vector)
    {
        return MovePtr(vector->data, sizeof(CanaryType), 1);
    }

    static inline ElemType* GetFirstCanaryAdr(const VectorType* const vector)
    {
        return MovePtr(vector->data, sizeof(CanaryType), -1);
    }

    static inline ElemType* GetSecondCanaryAdr(const VectorType* const vector)
    {
        return (ElemType*)((char*)(vector->data + vector->capacity) +
                           Aligning - (vector->capacity * sizeof(ElemType)) % Aligning);
    }

#endif

//-----------HASH PROTECTION---------------

#ifdef VECTOR_HASH_PROTECTION

    static inline HashType CalcDataHash(const VectorType* vector)
    {
        return vector->HashFunc(vector->data, vector->capacity * sizeof(ElemType), 0);        
    }

    static inline void UpdateDataHash(VectorType* vector)
    {
        vector->dataHash = CalcDataHash(vector);      
    }

    static inline void UpdateStructHash(VectorType* vector)
    {
        vector->structHash = 0;                                  
        vector->structHash = MurmurHash(vector, sizeof(*vector));        
    }
    
#endif

//--------------Consts-----------------

static const size_t STANDARD_CAPACITY = 64;

//---------------

#ifndef NDEBUG

    #define VECTOR_CHECK(vector)                    \
    do                                          \
    {                                           \
        VectorErrors vectorErr = VectorVerify(vector); \
                                                \
        if (vectorErr != VectorErrors::VECTOR_NO_ERR)                      \
        {                                       \
            VECTOR_DUMP(vector);                    \
            return vectorErr;                    \
        }                                       \
    } while (0)

    #define VECTOR_CHECK_NO_RETURN(vector)         \
    do                                         \
    {                                          \
        VectorErrors vectorErr = VectorVerify(vector);     \
                                                        \
        if (vectorErr != VectorErrors::VECTOR_NO_ERR)   \
        {                                               \
            VECTOR_DUMP(vector);                   \
        }                                      \
    } while (0)

#else

    #define VECTOR_CHECK(vector)           
    #define VECTOR_CHECK_NO_RETURN(vector) 

#endif

//---------------

#define IF_ERR_RETURN(ERR)              \
do                                      \
{                                       \
    if (ERR != VectorErrors::VECTOR_NO_ERR)                            \
        return ERR;                     \
} while (0)

//---------------

#ifdef VECTOR_HASH_PROTECTION
VectorErrors VectorCtor(VectorType* const vector, const size_t capacity, 
                          const HashFuncType HashFunc)
{
    assert(vector);

    ON_HASH
    (
        vector->HashFunc = HashFunc;
    )

    //--------SET STRUCT CANARY-------
    ON_CANARY
    (
        vector->structCanaryLeft  = Canary;
        vector->structCanaryRight = Canary;
    )

    VectorErrors errors = VectorErrors::VECTOR_NO_ERR;
    vector->size = 0;

    if (capacity > 0) vector->capacity = capacity;
    else              vector->capacity = STANDARD_CAPACITY;

    //----Callocing Memory-------

    vector->data = (ElemType*) calloc(VectorGetSizeForCalloc(vector), sizeof(*vector->data));

    if (vector->data == nullptr)
    {
        VectorPrintError(VectorErrors::VECTOR_MEMORY_ALLOCATION_ERROR);  
        return VectorErrors::VECTOR_MEMORY_ALLOCATION_ERROR;   
    }

    //-----------------------

    VectorDataFill(vector);

    ON_CANARY
    (
        vector->data = GetAfterFirstCanaryAdr(vector);
    )

    ON_HASH
    (
        UpdateDataHash(vector);
        UpdateStructHash(vector);
    )

    VECTOR_CHECK(vector);

    return errors;
}
#else 
VectorErrors VectorCtor(VectorType* const vector, const size_t capacity)
{
    assert(vector);

    //--------SET STRUCT CANARY-------
    ON_CANARY
    (
        vector->structCanaryLeft  = Canary;
        vector->structCanaryRight = Canary;
    )

    VectorErrors errors = VectorErrors::VECTOR_NO_ERR;
    vector->size = 0;

    if (capacity > 0) vector->capacity = capacity;
    else              vector->capacity = STANDARD_CAPACITY;

    //----Callocing Memory-------

    vector->data = (ElemType*) calloc(VectorGetSizeForCalloc(vector), sizeof(*vector->data));

    if (vector->data == nullptr)
    {
        VectorPrintError(VectorErrors::VECTOR_MEMORY_ALLOCATION_ERROR);  
        return VectorErrors::VECTOR_MEMORY_ALLOCATION_ERROR;   
    }

    //-----------------------

    VectorDataFill(vector);

    ON_CANARY
    (
        vector->data = GetAfterFirstCanaryAdr(vector);
    )

    VECTOR_CHECK(vector);

    return errors;    
}
#endif

VectorErrors VectorDtor(VectorType* const vector)
{
    assert(vector);

    VECTOR_CHECK(vector);
    
    for (size_t i = 0; i < vector->size; ++i)
    {
        vector->data[i] = POISON;
    }

    ON_CANARY
    (
        vector->data = GetFirstCanaryAdr(vector);
    )

    free(vector->data);
    vector->data = nullptr;

    vector->size     = 0;
    vector->capacity = 0;

    ON_HASH
    (
        vector->dataHash = 0;
    )

    ON_CANARY
    (
        vector->structCanaryLeft  = 0;
        vector->structCanaryRight = 0;
    )

    return VectorErrors::VECTOR_NO_ERR;
}

VectorErrors VectorPush(VectorType* vector, const ElemType val)
{
    assert(vector);
    
    VECTOR_CHECK(vector);

    VectorErrors vectorReallocErr = VectorErrors::VECTOR_NO_ERR;
    if (VectorIsFull(vector)) vectorReallocErr = VectorRealloc(vector, true);

    IF_ERR_RETURN(vectorReallocErr);

    vector->data[vector->size++] = val;

    ON_HASH
    (
        UpdateDataHash(vector);
        UpdateStructHash(vector);
    )

    VECTOR_CHECK(vector);

    return VectorErrors::VECTOR_NO_ERR;
}

VectorErrors VectorPop(VectorType* vector, ElemType* retVal)
{
    assert(vector);

    VECTOR_CHECK(vector);

    if (VectorIsEmpty(vector))
    { 
        VectorPrintError(VectorErrors::VECTOR_EMPTY_ERR);
        return VectorErrors::VECTOR_EMPTY_ERR;
    }

    --vector->size;
    if (retVal) *retVal = CpyFunc(&vector->data[vector->size]);
    vector->data[vector->size] = POISON;

    ON_HASH
    (
        UpdateDataHash(vector);
        UpdateStructHash(vector);
    )

    if (VectorIsTooBig(vector))
    {
        VectorErrors vectorReallocErr = VectorRealloc(vector, false);

        VECTOR_CHECK(vector);

        IF_ERR_RETURN(vectorReallocErr);

        ON_HASH
        (
            UpdateDataHash(vector);
            UpdateStructHash(vector);
        )
    }

    VECTOR_CHECK(vector);

    return VectorErrors::VECTOR_NO_ERR;
}

VectorErrors VectorVerify(VectorType* vector)
{
    assert(vector);

    if (vector->data == nullptr)
    {
        VectorPrintError(VectorErrors::VECTOR_IS_NULLPTR);
        return VectorErrors::VECTOR_IS_NULLPTR;
    }

    if (vector->capacity <= 0)
    {  
        VectorPrintError(VectorErrors::VECTOR_CAPACITY_OUT_OF_RANGE);
        return VectorErrors::VECTOR_CAPACITY_OUT_OF_RANGE;
    }

    if (vector->size > vector->capacity)
    {
        VectorPrintError(VectorErrors::VECTOR_SIZE_OUT_OF_RANGE);
        return VectorErrors::VECTOR_SIZE_OUT_OF_RANGE;
    }

    //-----------Canary checking----------

    ON_CANARY
    (
        if (*(CanaryType*)(GetFirstCanaryAdr(vector)) != Canary)
        {
            VectorPrintError(VectorErrors::VECTOR_INVALID_CANARY);
            return VectorErrors::VECTOR_INVALID_CANARY;
        }

        if (*(CanaryType*)(GetSecondCanaryAdr(vector)) != Canary)
        {
            VectorPrintError(VectorErrors::VECTOR_INVALID_CANARY);
            return VectorErrors::VECTOR_INVALID_CANARY;
        }

        if (vector->structCanaryLeft != Canary)
        {
            VectorPrintError(VectorErrors::VECTOR_INVALID_CANARY);
            return VectorErrors::VECTOR_INVALID_CANARY;
        }

        if (vector->structCanaryRight != Canary)
        {
            VectorPrintError(VectorErrors::VECTOR_INVALID_CANARY);
            return VectorErrors::VECTOR_INVALID_CANARY;
        }

    )

    //------------Hash checking----------

    ON_HASH
    (
        if (CalcDataHash(vector) != vector->dataHash)
        {
            VectorPrintError(VectorErrors::VECTOR_INVALID_DATA_HASH);
            return VectorErrors::VECTOR_INVALID_DATA_HASH;
        }
    )

    ON_HASH
    (
        if (CalcDataHash(vector) != vector->dataHash)
        {
            VectorPrintError(VectorErrors::VECTOR_INVALID_DATA_HASH);
            return VectorErrors::VECTOR_INVALID_DATA_HASH;
        }

        HashType prevStructHash = vector->structHash;
        UpdateStructHash(vector);

        if (prevStructHash != vector->structHash)
        {
            VectorPrintError(VectorErrors::VECTOR_INVALID_STRUCT_HASH);
            vector->structHash = prevStructHash;
            return VectorErrors::VECTOR_INVALID_STRUCT_HASH;
        }
    )


    return VectorErrors::VECTOR_NO_ERR;
}

void VectorDump(const VectorType* vector, const char* const fileName, 
                                     const char* const funcName,
                                     const int lineNumber)
{
    assert(vector);
    assert(fileName);
    assert(funcName);
    assert(lineNumber > 0);
    
    LOG_BEGIN();

    Log("VectorDump was called in file %s, function %s, line %d\n", fileName, funcName, lineNumber);
    Log("vector[%p]\n{\n", vector);
    Log("\tvector capacity: %zu, \n"
        "\tvector size    : %zu,\n",
        vector->capacity, vector->size);
    
    ON_CANARY
    (
        Log("\tLeft struct canary : " CanaryTypeFormat ",\n", 
            vector->structCanaryLeft);
        Log("\tRight struct canary: " CanaryTypeFormat ",\n", 
            vector->structCanaryRight);

        Log("\tLeft data canary : " CanaryTypeFormat ",\n", 
            *(CanaryType*)(GetFirstCanaryAdr(vector)));
        Log("\tRight data canary: " CanaryTypeFormat ",\n", 
            *(CanaryType*)(GetSecondCanaryAdr(vector)));
    )

    ON_HASH
    (
        Log("\tData hash  : %llu\n", vector->dataHash);
        Log("\tStruct hash: %llu\n", vector->structHash);
    )

    Log("\tdata data[%p]\n\t{\n", vector->data);

    /*
    if (vector->data != nullptr)
    {
        for (size_t i = 0; i < (vector->size < vector->capacity ? vector->size : vector->capacity); ++i)
        {
            //Log("\t\t*[%zu] = " ElemTypeFormat, i, vector->data[i]);

            if (Equal(&vector->data[i], &POISON)) Log(" (POISON)");

            Log("\n");
        }

        Log("\t\tNot used values:\n");

        for(size_t i = vector->size; i < vector->capacity; ++i)
        {
            //Log("\t\t*[%zu] = " ElemTypeFormat, i, vector->data[i]);
            
            //if (Equal(&vector->data[i], &POISON)) Log(" (POISON)");

            Log("\n");
        }
    }
    */

    Log("\t}\n}\n");

    LOG_END();
}

VectorErrors VectorRealloc(VectorType* vector, bool increase)
{
    assert(vector);

    VECTOR_CHECK(vector);
    
    if (increase) vector->capacity <<= 1;
    else          vector->capacity >>= 1;

    if (!increase) 
        FillArray(vector->data + vector->capacity, vector->data + vector->size, POISON);

    //--------Moves data to the first canary-------
    ON_CANARY
    (
        vector->data = GetFirstCanaryAdr(vector);
    )

    ElemType* tmpVector = (ElemType*) realloc(vector->data, 
                                             VectorGetSizeForCalloc(vector) * sizeof(*vector->data));

    if (tmpVector == nullptr)
    {
        VectorPrintError(VectorErrors::VECTOR_MEMORY_ALLOCATION_ERROR);

        assert(vector);
        if (increase) vector->capacity >>= 1;
        else          vector->capacity <<= 1;

        VECTOR_CHECK(vector);

        return VectorErrors::VECTOR_NO_ERR;
    }

    vector->data = tmpVector;

    // -------Moving forward after reallocing-------
    ON_CANARY
    (
        vector->data = GetAfterFirstCanaryAdr(vector);
    )

    if (increase)
        FillArray(vector->data + vector->size, vector->data + vector->capacity, POISON);

    // -------Putting canary at the end-----------
    ON_CANARY
    (
        CanaryCtor(GetSecondCanaryAdr(vector));
    )

    ON_HASH
    (
        UpdateDataHash(vector);
        UpdateStructHash(vector);
    )
    
    VECTOR_CHECK(vector);

    return VectorErrors::VECTOR_NO_ERR;
}

static inline bool VectorIsFull(VectorType* vector)
{
    assert(vector);

    VECTOR_CHECK_NO_RETURN(vector);

    return vector->size >= vector->capacity;
}

static inline bool VectorIsTooBig(VectorType* vector)
{
    assert(vector);

    VECTOR_CHECK_NO_RETURN(vector);

    return (vector->size * 4 <= vector->capacity) & (vector->capacity > STANDARD_CAPACITY);
}

static inline ElemType* MovePtr(ElemType* const data, const size_t moveSz, const int times)
{
    assert(data);
    assert(moveSz > 0);
    
    return (ElemType*)((char*)data + times * (long long)moveSz);
}

// NO vector check because doesn't fill hashes
static void VectorDataFill(VectorType* const vector)
{
    assert(vector);
    assert(vector->data);
    assert(vector->capacity > 0);

    ON_CANARY
    (
        CanaryCtor(vector->data);
        vector->data = GetAfterFirstCanaryAdr(vector);
    )

    FillArray(vector->data, vector->data + vector->capacity, POISON);

    ON_CANARY
    (
        CanaryCtor(GetSecondCanaryAdr(vector));
        vector->data = GetFirstCanaryAdr(vector);
    )

    // No vector check because doesn't fill hashes
}

// no VECTOR_CHECK because can be used for callocing memory (data could be nullptr at this moment)
static inline size_t VectorGetSizeForCalloc(VectorType* const vector)
{
    assert(vector);
    assert(vector->capacity > 0);

    ON_CANARY
    (
        return vector->capacity + 3 * sizeof(CanaryType) / sizeof(*vector->data);
    )

    return vector->capacity;
}

#undef VECTOR_CHECK
#undef IF_ERR_RETURN
#undef GetAfterFirstCanaryAdr
#undef GetFirstCanaryAdr
#undef GetSecondCanaryAdr

#define LOG_ERR(X) Log(HTML_RED_HEAD_BEGIN "\n" X "\n" HTML_HEAD_END "\n")
void VectorPrintError(VectorErrors error)
{
    LOG_BEGIN();

    switch(error)
    {
        case VectorErrors::VECTOR_CAPACITY_OUT_OF_RANGE:
            LOG_ERR("Vector capacity is out of range.\n");
            break;
        case VectorErrors::VECTOR_IS_NULLPTR:
            LOG_ERR("Vector is nullptr.\n");
            break;
        case VectorErrors::VECTOR_EMPTY_ERR:
            LOG_ERR("Trying to pop from empty vector.\n");
            break;
        case VectorErrors::VECTOR_SIZE_OUT_OF_RANGE:
            LOG_ERR("Vector size is out of range.\n");
            break;
        case VectorErrors::VECTOR_MEMORY_ALLOCATION_ERROR:
            LOG_ERR("Couldn't allocate more memory for vector.\n");
            break;
        case VectorErrors::VECTOR_INVALID_CANARY:
            LOG_ERR("Vector canary is invalid.\n");
            break;
        case VectorErrors::VECTOR_INVALID_DATA_HASH:
            LOG_ERR("Vector data hash is invalid.\n");
            break;
        case VectorErrors::VECTOR_INVALID_STRUCT_HASH:
            LOG_ERR("Vector struct hash is invalid.\n");

        case VectorErrors::VECTOR_NO_ERR:
        default:
            break;
    }

    LOG_END();
}
#undef PRINT_ERR
