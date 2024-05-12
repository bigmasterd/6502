#ifndef MEM_H
#define MEM_H

#include <stdio.h>
#include "types.h"

//6502 has 256 pages of RAM, each page is 256 bytes => 64k (65536) bytes overall
#define MEMSIZE 256*256

typedef word* TMemory;

//allocate RAM and return pointer to it
TMemory memInit(void);

//read 8bit word from 16bit address a
word memRead(TMemory mem, address a);

//write 8bit word to 16bit address a
void memWrite(TMemory mem, word w, address a);

//print RAM contents for memory in range [from,to]
void memDump(TMemory mem, address from, address to);

#endif
