#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

enum class CommandsErrors
{
    NO_ERR,

    INVALID_COMMAND_SYNTAX,
    INVALID_COMMAND_STRING,
    INVALID_COMMAND_ID,

    INVALID_SIGNATURE,
    INVALID_VERSIONS,
    INVALID_ADDED_INFO,
};

typedef int SignatureType;
typedef int VersionType;

#pragma GCC diagnostic ignored "-Wmultichar"
static const SignatureType Signature = 'COCK';
#pragma GCC diagnostic warning "-Wmultichar"

static const int ARG_FORMAT_IMM = 0x20;
static const int ARG_FORMAT_REG = 0x40;
static const int ARG_FORMAT_RAM = 0x80;

static const size_t NumberOfRegisters = 26;
static const size_t RamSize           = 1000;

#define DEF_CMD(NAME, NUM, ...)     \
    NAME = NUM,         

enum class Commands
{
    #include "Commands.h"
};

#undef DEF_CMD

#endif // COMMON_H
