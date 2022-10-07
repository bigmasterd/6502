#include "types.h"

void printRegs(void);

int load(const char* file);

address lohi2addr(word lo, word hi);

word getBit(word w, uint32_t bit_number);

void setBit(word* w, uint32_t bit_number);

void clearBit(word* w, uint32_t bit_number);