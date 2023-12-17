//DEF_CMD(NAME, ENUM_ID, USE_LABELS, SPU_CODE, SWITH_CASE_SPU_CALL,)


DEF_CMD(PUSH, 0, false,
static SpuErrors CommandPush(SpuType* spu, int command)
{
    assert(spu);

    SPU_CHECK(spu);

    StackPush(&spu->stack, *GetArgument(spu, command));

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
},
{
    spuError = CommandPush(spu, command);
}
)

DEF_CMD(POP, 1, false,
static SpuErrors CommandPop(SpuType* spu, int command)
{
    assert(spu);

    SPU_CHECK(spu);

    StackPop(&spu->stack, GetArgument(spu, command));

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
},
{
    spuError = CommandPop(spu, command);
}
)

DEF_CMD(MUL, 2, false,
static inline SpuErrors CommandMul(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = (1.0 * inFirstValue / CalculatingPrecision) * inSecondValue;

    return SpuErrors::NO_ERR;
},
{
    spuError = CallBinaryCommand(CommandMul, spu);
})

DEF_CMD(ADD, 3, false,
static inline SpuErrors CommandAdd(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue + inSecondValue;

    return SpuErrors::NO_ERR;
},
{
    spuError = CallBinaryCommand(CommandAdd, spu);
}
)

DEF_CMD(DIV, 4, false,
static inline SpuErrors CommandDiv(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    if (inSecondValue == 0)
    {
        *outValue = POISON;
        SPU_ERRORS_LOG_ERROR(SpuErrors::TRYING_TO_DIVIDE_ON_ZERO);
                      return SpuErrors::TRYING_TO_DIVIDE_ON_ZERO;
    }

    *outValue = CalculatingPrecision * inFirstValue / inSecondValue;
    
    return SpuErrors::NO_ERR;
},
{
    spuError = CallBinaryCommand(CommandDiv, spu);
}
)

DEF_CMD(SUB, 5, false,
static inline SpuErrors CommandSub(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue - inSecondValue;

    return SpuErrors::NO_ERR;
},
{
    spuError = CallBinaryCommand(CommandSub, spu);
}
)

DEF_CMD(SIN, 6, false,
static inline SpuErrors CommandSin(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(sin(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
},
{
    spuError = CallUnaryCommand(CommandSin, spu);
})

DEF_CMD(COS, 7, false,
static inline SpuErrors CommandCos(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(cos(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
},
{
    spuError = CallUnaryCommand(CommandCos, spu);
},)

DEF_CMD(TAN, 8, false,
static inline SpuErrors CommandTan(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(tan(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
},
{
    spuError = CallUnaryCommand(CommandTan, spu);
}
)

DEF_CMD(COT, 9, false,
static inline SpuErrors CommandCot(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(CalculatingPrecision / tan(1.0 * inValue / CalculatingPrecision));

    return SpuErrors::NO_ERR;
},
{
    spuError = CallUnaryCommand(CommandCot, spu);
})

DEF_CMD(SQRT, 10, false,
static inline SpuErrors CommandSqrt(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(sqrt(inValue) * sqrt(CalculatingPrecision));

    return SpuErrors::NO_ERR;
},
{
    spuError = CallUnaryCommand(CommandSqrt, spu);
}
)

DEF_CMD(POW, 11, false,
static inline SpuErrors CommandPow(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = pow(1.0 * inFirstValue / CalculatingPrecision, 
                    1.0 * inSecondValue / CalculatingPrecision) * CalculatingPrecision;
    
    return SpuErrors::NO_ERR;
},
{
    spuError = CallBinaryCommand(CommandPow, spu);
}
)

DEF_CMD(OUT, 12, false,
static inline SpuErrors CommandOut(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    printf("Equation result: " "%lg" "\n", 1.0 * inValue / CalculatingPrecision);

    *outValue = inValue;

    return SpuErrors::NO_ERR;
},
{
    spuError = CallUnaryCommand(CommandOut, spu);
})

DEF_CMD(HLT, 13, false,
static inline void CommandHlt()
{
    return;
},
{
    break;
})

DEF_CMD(JMP, 14, true,
static inline SpuErrors CommandJmp(SpuType* spu, int command)
{
    assert(spu);

    SPU_CHECK(spu);

    //printf("H1\n");

    int* poss = GetArgument(spu, command);
    assert(poss);
    int posToJump = *poss;
    //printf("H2\n");

    assert(posToJump > 0);

    spu->ip = posToJump; 

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;    
},
{
    spuError = CommandJmp(spu, command);
})

#define COMMAND_JUMP_IF(comparisonSign)                                                         \
{                                                                                               \
    assert(spu);                                                                                \
                                                                                                \
    SPU_CHECK(spu);                                                                             \
                                                                                                \
    int firstValue  = POISON;                                                                   \
    int secondValue = POISON;                                                                   \
                                                                                                \
    SpuErrors error = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);        \
                                                                                                \
    IF_ERR_RETURN(error);                                                                       \
                                                                                                \
    if (firstValue comparisonSign secondValue)                                                  \
        error = CommandJmp(spu, command);                                                       \
    else                                                                                        \
        spu->ip++;                                                                              \
                                                                                                \
    return error;                                                                               \
}

DEF_CMD(JB, 15, true,
static inline SpuErrors CommandJb(SpuType* spu, int command)
{
    COMMAND_JUMP_IF(<);
},
{
    spuError = CommandJb(spu, command);
})

DEF_CMD(JA, 16, true,
static inline SpuErrors CommandJa(SpuType* spu, int command)
{
    COMMAND_JUMP_IF(>);
},
{
    spuError = CommandJa(spu, command);
})

DEF_CMD(JAE, 17, true,
static inline SpuErrors CommandJae(SpuType* spu, int command)
{
    COMMAND_JUMP_IF(>=);
},
{
    spuError = CommandJae(spu, command);
})

DEF_CMD(JBE, 18, true,
static inline SpuErrors CommandJbe(SpuType* spu, int command)
{
    COMMAND_JUMP_IF(<=);
},
{
    spuError = CommandJbe(spu, command);
})

DEF_CMD(JE, 19, true,
static inline SpuErrors CommandJe(SpuType* spu, int command)
{
    COMMAND_JUMP_IF(==);
},
{
    spuError = CommandJe(spu, command);
})

DEF_CMD(JNE, 20, true,
static inline SpuErrors CommandJne(SpuType* spu, int command)
{
    COMMAND_JUMP_IF(!=);
},
{
    spuError = CommandJne(spu, command);
})

#undef COMMAND_JUMP_IF

DEF_CMD(CALL, 21, true,
static inline SpuErrors CommandCall(SpuType* spu, int command)
{
    assert(spu);

    SPU_CHECK(spu);

    int posToJump = *GetArgument(spu, command);

    assert(posToJump > 0);

    StackPush(&spu->stackRet, spu->ip); 

    spu->ip = posToJump; 

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;  
},
{
    spuError = CommandCall(spu, command);
})

DEF_CMD(RET, 22, false,
static inline SpuErrors CommandRet(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int retAdr = -1;

    SpuErrors error = GetStackLastValue(&spu->stackRet, &retAdr);

    assert(retAdr >= 0);

    spu->ip = retAdr; 

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;  
},
{
    spuError = CommandRet(spu);
})

DEF_CMD(IN, 23, false,
static SpuErrors CommandIn(SpuType* spu)
{
    assert(spu);

    //printf("Z0\n");
    SPU_CHECK(spu);
    //printf("Z1\n");

    int valueToPush = POISON;
    
    printf("Enter value: ");
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);
    
    if (scanfResult != 1)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::READING_FROM_BYTE_CODE_ERR);
                      return SpuErrors::READING_FROM_BYTE_CODE_ERR;
    }
    
    valueToPush *= CalculatingPrecision;

    STACK_PUSH(spu, valueToPush);

    SPU_CHECK(spu);
    
    return SpuErrors::NO_ERR;
},
{
    //printf("K0\n");
    spuError = CommandIn(spu);
    //printf("K1\n");
})

DEF_CMD(OUTC, 24, false, 
static SpuErrors CommandOutc(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    printf("%c", (char)(inValue / CalculatingPrecision));

    *outValue = inValue;

    return SpuErrors::NO_ERR;
},
{
    spuError = CallUnaryCommand(CommandOutc, spu);
})
