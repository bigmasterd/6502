#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "6502.h"

void printRegs(T6502 cpu);

void printExecInfo(word opcode);

address lohi2addr(word lo, word hi);

word getBit(word w, uint32_t bit_number);

void setBit(word* w, uint32_t bit_number);

void clearBit(word* w, uint32_t bit_number);

#endif