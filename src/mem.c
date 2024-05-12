#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

TMemory memInit(void)
{
    TMemory mem = (word*)malloc(sizeof(word) * MEMSIZE);  
    memset(mem, 0, sizeof(word) * MEMSIZE);
    return mem;
}

//read 8bit word from 16bit address a
word memRead(TMemory mem, address a)
{
    return mem[a];
}

//write 8bit word to 16bit address a
void memWrite(TMemory mem, word w, address a)
{
    mem[a] = w;
}

//print RAM contents for memory in range [from,to]
void memDump(TMemory mem, address from, address to)
{
    address i = from;
    
    printf("\n**********************************************************************");   
    for (i = 0; i < to; i++)
    { 
        if ((i) % 16 == 0) printf("\n%.4X: ", i);        
        printf(" %.2X ", memRead(mem, i)); 
    }   
    printf("\n**********************************************************************\n\n");
}
