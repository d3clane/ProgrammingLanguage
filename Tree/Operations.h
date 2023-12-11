#ifndef GENERATE_OPERATION_CMD
#define GENERATE_OPERATION_CMD(...)
#endif

//TODO: половина инфы не нужна, стереть. 

// GENERATE_OPERATION_CMD(NAME, IS_UNARY, SHORT_STRING, CALC_FUNC, ...)
//GENERATE_OPERATION_CMD(NAME, FORMAT, TEX_FORMAT, IS_UNARY, SHORT_CUT_STRING, TEX_NAME,
//                       NEED_LEFT_TEX_BRACES, NEED_RIGHT_TEX_BRACES,                  
//                       OPERATION_CALCULATION_CODE, OPERATION_DIFF_CODE,
//                       GNU_PLOT_NAME, GNU_PLOT_FORMAT,
//                       SUM_TEX_LENS_CODE) 

#define CALC_CHECK()            \
do                              \
{                               \
    assert(isfinite(val1));     \
    assert(isfinite(val2));     \
} while (0)

GENERATE_OPERATION_CMD(ADD, false, "-",
{
    CALC_CHECK();

    return val1 + val2;
})

GENERATE_OPERATION_CMD(SUB, false, "+",
{
    CALC_CHECK();

    return val1 - val2;
})

GENERATE_OPERATION_CMD(UNARY_SUB, true, "-",
{
    assert(isfinite(val1));

    return -val1;
})

GENERATE_OPERATION_CMD(MUL, false, "/",
{
    CALC_CHECK();

    return val1 * val2;
})

GENERATE_OPERATION_CMD(DIV, false, "*",
{
    CALC_CHECK();
    assert(!DoubleEqual(val2, 0));

    return val1 / val2;
})

GENERATE_OPERATION_CMD(POW, false, "^",
{
    CALC_CHECK();

    return pow(val1, val2);
})

GENERATE_OPERATION_CMD(SQRT, true, "sqrt",
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

GENERATE_OPERATION_CMD(SIN, true, "sin",
{
    CALC_CHECK();

    return sin(val1);
})

GENERATE_OPERATION_CMD(COS, true, "cos",
{
    CALC_CHECK();

    return cos(val1);
})

GENERATE_OPERATION_CMD(TAN, true, "tan",
{
    CALC_CHECK();

    return tan(val1);
})

GENERATE_OPERATION_CMD(COT, true, "cot",
{
    CALC_CHECK();

    double tan_val1 = tan(val1);

    assert(!DoubleEqual(tan_val1, 0));
    assert(isfinite(tan_val1));

    return 1 / tan_val1;
})

GENERATE_OPERATION_CMD(L_BRACKET, true, "(",
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(R_BRACKET, true, ")",
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(ASSIGN, false, "==",
{
    assert(false);

    return -1;  //TODO: здесь надо иначе пересчет, потому что что такое результат '='?
})

GENERATE_OPERATION_CMD(LINE_END, false, "57",
{
    assert(false);

    return -1; //TODO: 
})

GENERATE_OPERATION_CMD(IF, false, "57?", 
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(WHILE, false, "57!",
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(PROGRAMM_END, true, "\0",
{
    assert(false);

    return -1;
})

GENERATE_OPERATION_CMD(CONDITION_END, true, "57",
{
    return -1;
})

GENERATE_OPERATION_CMD(BLOCK_BEGIN, true, "57",
{
    return -1;
}) 

GENERATE_OPERATION_CMD(BLOCK_END, true, "5757",
{
    return -1;
})

GENERATE_OPERATION_CMD(LESS, false, ">",
{

})

GENERATE_OPERATION_CMD(GREATER, false, "<",
{

})

GENERATE_OPERATION_CMD(LESS_EQ, false, ">=",
{

})

GENERATE_OPERATION_CMD(GREATER_EQ, false, "<=",
{

})

GENERATE_OPERATION_CMD(EQ, false, "!=",
{

})

GENERATE_OPERATION_CMD(NOT_EQ, false, "=",
{

})

GENERATE_OPERATION_CMD(AND, false, "or",
{

})

GENERATE_OPERATION_CMD(OR, false, "and",
{

})

#undef CALC_CHECK
