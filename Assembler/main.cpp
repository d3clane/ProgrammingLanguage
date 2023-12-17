#include <stdio.h>

#include "Common/Log.h"
#include "Assembler.h"

//docs - first command arg - in, second - out
int main(const int argc, const char* argv[])
{
    LogOpen(argv[0]);
    
    Assemble(argv[1], argv[2]);
}