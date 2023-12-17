#ifndef GENERATE_OPERATION_CMD
#define GENERATE_OPERATION_CMD(...)
#endif

//TODO: половина инфы не нужна, стереть. 

// GENERATE_OPERATION_CMD(NAME, IS_UNARY, SHORT_STRING, CALC_FUNC, ...)

#define CALC_CHECK()            \
do                              \
{                               \
    assert(isfinite(val1));     \
    assert(isfinite(val2));     \
} while (0)

GENERATE_OPERATION_CMD(ADD, false,
{
    CALC_CHECK();

    return val1 + val2;
})

GENERATE_OPERATION_CMD(SUB, false,
{
    CALC_CHECK();

    return val1 - val2;
})

GENERATE_OPERATION_CMD(UNARY_SUB, true,
{
    assert(isfinite(val1));

    return -val1;
})

GENERATE_OPERATION_CMD(MUL, false,
{
    CALC_CHECK();

    return val1 * val2;
})

GENERATE_OPERATION_CMD(DIV, false,
{
    CALC_CHECK();
    assert(!DoubleEqual(val2, 0));

    return val1 / val2;
})

GENERATE_OPERATION_CMD(POW, false,
{
    CALC_CHECK();

    return pow(val1, val2);
})

GENERATE_OPERATION_CMD(SQRT, true,
{
    CALC_CHECK();

    assert(val1 > 0); //TODO: надо бы сравнение даблов сделать

    return sqrt(val1);
})

#undef  CALC_CHECK
#define CALC_CHECK()        \
do                          \
{                           \
    assert(isfinite(val1)); \
} while (0)

GENERATE_OPERATION_CMD(SIN, true,
{
    CALC_CHECK();

    return sin(val1);
})

GENERATE_OPERATION_CMD(COS, true,
{
    CALC_CHECK();

    return cos(val1);
})

GENERATE_OPERATION_CMD(TAN, true,
{
    CALC_CHECK();

    return tan(val1);
})

GENERATE_OPERATION_CMD(COT, true,
{
    CALC_CHECK();

    double tan_val1 = tan(val1);

    assert(!DoubleEqual(tan_val1, 0));
    assert(isfinite(tan_val1));

    return 1 / tan_val1;
})

GENERATE_OPERATION_CMD(ASSIGN, false,
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(LINE_END, false, 
{
    assert(false);

    return -1; //TODO: 
})

GENERATE_OPERATION_CMD(IF, false, 
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(WHILE, false,
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(LESS, false, 
{

})

GENERATE_OPERATION_CMD(GREATER, false, 
{

})

GENERATE_OPERATION_CMD(LESS_EQ, false, 
{

})

GENERATE_OPERATION_CMD(GREATER_EQ, false,
{

})

GENERATE_OPERATION_CMD(EQ, false, 
{

})

GENERATE_OPERATION_CMD(NOT_EQ, false,
{

})

GENERATE_OPERATION_CMD(AND, false,
{

})

GENERATE_OPERATION_CMD(OR, false,
{

})

//TODO: PRINT -> '{'
GENERATE_OPERATION_CMD(PRINT, true,
{

})

//TODO: READ -> '{'
GENERATE_OPERATION_CMD(READ, true,
{

})

GENERATE_OPERATION_CMD(COMMA, false,
{

})

GENERATE_OPERATION_CMD(TYPE_INT, false,
{

})

GENERATE_OPERATION_CMD(TYPE, false,
{

})

GENERATE_OPERATION_CMD(NEW_FUNC, false,
{

})

GENERATE_OPERATION_CMD(FUNC, false,
{

})

GENERATE_OPERATION_CMD(FUNC_CALL, false,
{

})

GENERATE_OPERATION_CMD(RETURN, false,
{

})

#undef CALC_CHECK