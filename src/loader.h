#ifndef LOADER_H
#define LOADER_H

#include "mem.h"

//load 6202 binary from specified byte array
int loadProgram(TMemory mem, const word* program, uint32_t length);

//load 6202 binary from file
int loadProgramFromFile(TMemory mem, const char* file);

#endif