#include <stdio.h>
#include "mem.h"

#define MEMSIZE 256*256 //256 pages a 256 bytes = 64k = 65536 bytes memory


word mem[MEMSIZE]; 


word mrd(address a)
{
	return mem[a];
}


void mwr(word w, address a)
{
	mem[a] = w;
}


void mdump(address from, address to)
{
    address i = from;
    
	printf("\n");
	printf("**********************************************************************");	
    for (i = 0; i < to; i++)
    { 
        if ((i) % 16 == 0) printf("\n%.4X: ", i);        
        printf(" %.2X ", mrd(i)); 
    }	
	printf("\n**********************************************************************\n");
    printf("\n");
}
