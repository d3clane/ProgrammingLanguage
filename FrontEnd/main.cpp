#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"
#include "FastInput/InputOutput.h"

int main()
{
    FILE* inStream = fopen("code.txt", "r");

    char* inputTxt = ReadText(inStream);

    CodeParse(inputTxt);

    free(inputTxt);
}