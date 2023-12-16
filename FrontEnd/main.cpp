#include <stdio.h>
#include <stdlib.h>

#include "Common/Log.h"
#include "SyntaxParser.h"
#include "FastInput/InputOutput.h"

int main(int argc, char* argv[])
{
    LogOpen(argv[0]);
    setbuf(stdout, nullptr);

    FILE* inStream = fopen("code.txt", "r");

    char* inputTxt = ReadText(inStream);

    SyntaxParserErrors err = SyntaxParserErrors::NO_ERR;
    CodeParse(inputTxt, &err);

    free(inputTxt);

    return (int)err;
}
