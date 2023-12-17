#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Assembler.h"
#include "InputOutput/InputOutput.h"
#include "Common/StringFuncs.h"
#include "Common/Common.h"

static char* ReadRegister(const char* inCode, size_t* inPos);
static bool ReadNumValue(const char* inCode, size_t* inPos, int* value);
static int  GetRegisterId    (const char* reg);

struct LabelType
{
    int     jmpAdress;
    char*   labelName;
};

struct LabelTypeArr
{
    LabelType* data;

    size_t size;
    size_t capacity;
};

static AssemblerErrors BuildByteCode(const char* inCode, int** outCode, size_t* outCodeSz);

static inline LabelType* AddLabel (LabelTypeArr* arr, const char* labelName, 
                                                      const int   labelAdress);
static inline LabelType* GetLabel (const LabelTypeArr* arr, const char* labelName);
static inline bool       IsLabel  (const char* labelName);
static inline void LabelTypeDtor  (LabelType* label);
static inline void LabelsArrayCtor(LabelTypeArr* arr, const size_t capacity = 100);
static inline void LabelsArrayDtor(LabelTypeArr* arr);

static AssemblerErrors LabelsArrayBuild(LabelTypeArr* labelsArr, const char* inCode, 
                                                                        int* outCode);
static AssemblerErrors AsseemblerParseCommand(const char* command, const size_t maxCommandLength,
                                              const char* inCode,        size_t* inCodePos,
                                              int* outCode,              size_t* outCodePos,
                                              const LabelTypeArr* labelsArr);
static AssemblerErrors CommandsParse(const char* inCode, int* outCode, size_t* outCodeSz,
                                     const LabelTypeArr* labelsArr);
static AssemblerErrors AssemblerParseCommandWithLabels
                                        (const char* command, const size_t maxCommandLength,
                                         const char* inCode,        size_t* inCodePos,
                                         int* outCode,              size_t* outCodePos,
                                         const LabelTypeArr* labelsArr);
static AssemblerErrors AssemblerParseCommandWithoutLabels
                                            (const char* command, const size_t maxCommandLength,
                                             const char* inCode,        size_t* inCodePos, 
                                             int* outCode,              size_t* outCodePos);

AssemblerErrors Assemble(const char* inStreamName, const char* outStreamName)
{
    assert(inStreamName);
    assert(outStreamName);

    FILE* inStream  = fopen(inStreamName,  "r");
    FILE* outStream = fopen(outStreamName, "w");

    AssemblerErrors err = Assemble(inStream, outStream);

    fclose(inStream);
    fclose(outStream);

    return err;
}

AssemblerErrors Assemble(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    char* inCode    = ReadText(inStream);

    AssemblerErrors err = AssemblerErrors::NO_ERR;

    int* outCode     = nullptr;
    size_t outCodeSz = 0;

    BuildByteCode(inCode, &outCode, &outCodeSz);

    fwrite(outCode, sizeof(*outCode), outCodeSz, outStream);

    free(inCode);
    free(outCode);

    return err;
}

static AssemblerErrors BuildByteCode(const char* inCode, int** outCode, size_t* outCodeSz)
{
    assert(inCode);
    assert(outCode);
    assert(outCodeSz);

    *outCode      = (int*) calloc(strlen(inCode), sizeof(**outCode));

    AssemblerErrors err = AssemblerErrors::NO_ERR;

    LabelTypeArr labelsArr = {};
    err = LabelsArrayBuild(&labelsArr, inCode, *outCode);
    err = CommandsParse(inCode, *outCode, outCodeSz, &labelsArr);

    LabelsArrayDtor(&labelsArr);
    
    return err;
}

static AssemblerErrors CommandsParse(const char* inCode, int* outCode, size_t* outCodeSz,
                                     const LabelTypeArr* labelsArr)
{
    assert(inCode);
    assert(outCode);
    assert(outCodeSz);
    assert(labelsArr);

    size_t inPos  = 0;
    size_t outPos = 0;

    static const size_t maxCommandLength  = 64;
    static char command[maxCommandLength] = "";

    AssemblerErrors err = AssemblerErrors::NO_ERR;

    while (inCode[inPos])
    {
        int shift = 0;
        sscanf(inCode + inPos, "%s%n", command, &shift);
        inPos += shift;

        if (IsLabel(command))
            continue;

        err = AsseemblerParseCommand(command, maxCommandLength, inCode,  &inPos, 
                                     outCode, &outPos, labelsArr);
    }

    *outCodeSz = outPos;

    return err;
}

static AssemblerErrors LabelsArrayBuild(LabelTypeArr* labelsArr, const char* inCode, 
                                                                        int* outCode)
{
    assert(labelsArr);
    assert(inCode);
    assert(outCode);

    LabelsArrayCtor(labelsArr);

    size_t inPos  = 0;
    size_t outPos = 0;

    static const size_t maxCommandLength  = 64;
    static char command[maxCommandLength] = "";

    AssemblerErrors err = AssemblerErrors::NO_ERR;

    while (inCode[inPos])
    {
        int shift = 0;
        sscanf(inCode + inPos, "%s%n", command, &shift);
        inPos += shift;

        if (IsLabel(command))
        {
            LabelType* tmp = AddLabel(labelsArr, command, outPos);
            printf("NEW LABEL, name - %s, adr - %d\n", tmp->labelName, tmp->jmpAdress);
            continue;
        }

        err = AsseemblerParseCommand(command, maxCommandLength, inCode,  &inPos, 
                                     outCode, &outPos, labelsArr);
    }

    return err;
}

static AssemblerErrors AsseemblerParseCommand(const char* command, const size_t maxCommandLength,
                                              const char* inCode,        size_t* inCodePos,
                                              int* outCode,              size_t* outCodePos,
                                              const LabelTypeArr* labelsArr)
{
    assert(command);
    assert(inCode);
    assert(inCodePos);
    assert(outCode);
    assert(outCodePos);

    #define DEF_CMD(NAME, num, USE_LABELS, ...)                                             \
        if (strcasecmp(command, #NAME) == 0)                                                \
        {                                                                                   \
            if (USE_LABELS)                                                                 \
                return AssemblerParseCommandWithLabels(command, maxCommandLength,           \
                                                 inCode, inCodePos, outCode, outCodePos,    \
                                                 labelsArr);                                \
            else                                                                            \
                return AssemblerParseCommandWithoutLabels(command, maxCommandLength,        \
                                                    inCode, inCodePos, outCode, outCodePos);\
        }                                                                                   \
        else 
    
    #include "Common/Commands.h"

    /* else */
    {
        assert(false);
        return AssemblerErrors::SYNTAX_ERR;
    }

    return AssemblerErrors::SYNTAX_ERR;
}

static AssemblerErrors AssemblerParseCommandWithLabels
                                        (const char* command, const size_t maxCommandLength,
                                         const char* inCode,        size_t* inCodePos,
                                         int* outCode,              size_t* outCodePos,
                                         const LabelTypeArr* labelsArr)
{
    assert(command);
    assert(inCode);
    assert(inCodePos);
    assert(outCode);
    assert(outCodePos);

    int jmpAdress = 0;
    int scanfResult = sscanf(inCode + *inCodePos, "%d", &jmpAdress);

    #define DEF_CMD(NAME, num, USE_LABELS, ...)             \
        if (USE_LABELS && strcasecmp(command, #NAME) == 0)  \
            outCode[*outCodePos] = num | ARG_FORMAT_IMM;    \
        else

    #include "Common/Commands.h"

    /* else */
    {
        assert(false);

        return AssemblerErrors::SYNTAX_ERR;
    }

    (*outCodePos)++;

    #undef DEF_CMD

    if (scanfResult != 0)
    {
        outCode[*outCodePos] = jmpAdress;
        (*outCodePos)++;

        return AssemblerErrors::NO_ERR;
    }

    static const size_t maxLabelLength  = 64;
    static char   label[maxLabelLength] = "";

    int shift = 0;
    sscanf(inCode + *inCodePos, "%s%n", label, &shift);
    *inCodePos += shift;

    LabelType* labelAdr = GetLabel(labelsArr, label);

    if (labelAdr == nullptr)
        outCode[*outCodePos] = -1;
    else
        outCode[*outCodePos] = labelAdr->jmpAdress;
    
    (*outCodePos)++;

    return AssemblerErrors::NO_ERR;
}

static AssemblerErrors AssemblerParseCommandWithoutLabels
                                            (const char* command, const size_t maxCommandLength,
                                             const char* inCode,        size_t* inCodePos, 
                                             int* outCode,              size_t* outCodePos)
{
    assert(command);
    assert(inCode);
    assert(inCodePos);
    assert(outCode);
    assert(outCodePos);

    // grammar : '[' -> reg -> plus -> num -> ']' 

    size_t inPos  = *inCodePos;
    size_t outPos = *outCodePos;
    
    int commandId  = 0;
    #define DEF_CMD(NAME, num, ...)                 \
        if (strcasecmp(#NAME, command) == 0)        \
            commandId = num;                        \
        else                                        \
    
    #include "Common/Commands.h"

    /* else */
    {
        assert(false);
        //TODO: написать строчку с ошибкой мб
        return AssemblerErrors::SYNTAX_ERR;
    }

    #undef DEF_CMD

    inPos = SkipSymbolsWhileStatement(inCode + inPos, isspace) - inCode;

    bool isRam = (inCode[inPos] == '[');

    if (isRam)
    {
        commandId |= ARG_FORMAT_RAM;
        ++inPos;
    }

    char* reg = ReadRegister(inCode, &inPos);
    int regId = -1;
    if (reg)
    {
        regId = GetRegisterId(reg);
        commandId |= ARG_FORMAT_REG;
    }

    int value = 0;
    bool hasValue = ReadNumValue(inCode, &inPos, &value);

    if (hasValue)
        commandId |= ARG_FORMAT_IMM;

    inPos = SkipSymbolsWhileStatement(inCode + inPos, isspace) - inCode;

    if (isRam)
    {
        assert(inCode[inPos] == ']');
        ++inPos;
    }

    printf("Command - %s, id - %d\n", command, commandId);
    outCode[outPos++] = commandId;

    if (hasValue)
        outCode[outPos++] = value;

    if (reg)
        outCode[outPos++] = regId; 
        
    if (reg)
        free(reg);

    *inCodePos  = inPos;
    *outCodePos = outPos;

    return AssemblerErrors::NO_ERR;
}   

static char* ReadRegister(const char* inCode, size_t* inPos)
{
    assert(inCode);
    assert(inPos);

    *inPos = SkipSymbolsWhileStatement(inCode + *inPos, isspace) - inCode;

    if (inCode[*inPos] != 'r' || inCode[*inPos + 2] != 'x' || 
        !('a' <= inCode[*inPos + 1] && inCode[*inPos + 1] <= 'z'))
        return nullptr;

    static const size_t registerLength  = 3;
    static char reg[registerLength + 1] = "";

    int shift = 0;
    sscanf(inCode + *inPos, "%c%c%c%n", reg, reg + 1, reg + 2, &shift);
    reg[registerLength] = '\0';

    *inPos += shift;

    return strdup(reg);
}

static bool ReadNumValue(const char* inCode, size_t* inPos, int* value)
{
    assert(inCode);
    assert(inPos);

    *inPos = SkipSymbolsWhileStatement(inCode + *inPos, isspace) - inCode;
    if (inCode[*inPos] == '+')
        (*inPos)++;
    
    *inPos = SkipSymbolsWhileStatement(inCode + *inPos, isspace) - inCode;
    int shift = 0;
    sscanf(inCode + *inPos, "%d%n", value, &shift);

    if (shift == 0)
        return false;

    *inPos += shift;

    return true;
}

static int GetRegisterId(const char* reg)
{
    assert(reg);

    if (reg[0] != 'r' || reg[2] != 'x' || !('a' <= reg[1] && reg[1] <= 'z'))
        return -1;

    return reg[1] - 'a';
}

static inline LabelType* GetLabel(const LabelTypeArr* arr, const char* labelName)
{
    assert(arr);
    assert(labelName);

    for (size_t i = 0; i < arr->size; ++i)
    {
        if (strcmp(labelName, arr->data[i].labelName) == 0)
            return arr->data + i;
    }

    return nullptr;
}

static inline LabelType* AddLabel(LabelTypeArr* arr, const char* labelName, 
                                                     const int   labelAdress)
{
    assert(arr);
    assert(labelName);

    if (!IsLabel(labelName))
        return nullptr;

    arr->data[arr->size].labelName = strdup(labelName);
    arr->data[arr->size].jmpAdress = labelAdress;
    arr->size++;

    return arr->data + arr->size - 1;
}

static inline bool IsLabel(const char* labelName)
{
    assert(labelName);

    const char* labelEndPtr = labelName + strlen(labelName) - 1;

    while (labelEndPtr > labelName)
    {
        if (*labelEndPtr == ':')
            return true;
        
        if (!isspace(*labelEndPtr))
            return false;

        labelEndPtr--;
    }

    return false;
}

static inline void LabelTypeDtor(LabelType* label)
{
    assert(label);

    label->jmpAdress = -1;

    if (label->labelName)
        free(label->labelName);

    label->labelName = nullptr;
}

static inline void LabelsArrayCtor(LabelTypeArr* arr, const size_t capacity)
{
    assert(arr);

    arr->data     = (LabelType*)calloc(capacity, sizeof(*arr->data));
    arr->capacity = capacity;
    arr->size     = 0;
}

static inline void LabelsArrayDtor(LabelTypeArr* arr)
{
    assert(arr);

    for (size_t i = 0; i < arr->size; ++i)
        LabelTypeDtor(arr->data + i);

    free(arr->data);
}
