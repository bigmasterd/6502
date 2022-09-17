#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "mem.h"

//6502 registers
extern word 	X;  
extern word 	Y;  
extern word 	A;  
extern word 	P;  
extern word 	IR; 
extern word  	SP; 
extern address  PC; 

//4bit integer to binary lookup table
const char* int2bin[] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};


void printRegs(void)
{
    printf("*** Register contents *** \n");
    printf("X:  0x%.2X (%s.%s) \n", X, int2bin[(X >> 4) & 0xF], int2bin[X & 0xF]);
    printf("Y:  0x%.2X (%s.%s) \n", Y, int2bin[(Y >> 4) & 0xF], int2bin[Y & 0xF]);    
    printf("A:  0x%.2X (%s.%s) \n", A, int2bin[(A >> 4) & 0xF], int2bin[A & 0xF]);
    printf("P:  0x%.2X N=%d,V=%d,B=%d,D=%d,I=%d,Z=%d,C=%d\n",  P, ((P >> 7) & 0x1), ((P >> 6) & 0x1), 
                ((P >> 4) & 0x1), ((P >> 3) & 0x1), ((P >> 2) & 0x1), ((P >> 1) & 0x1), ((P) & 0x1)); 
    printf("IR: 0x%.2X (%s.%s) \n", IR, int2bin[(IR >> 4) & 0xF], int2bin[IR & 0xF]);
    printf("SP: 0x%.2X (%s.%s) \n", SP, int2bin[(SP >> 4) & 0xF], int2bin[SP & 0xF]);
    printf("PC: 0x%.4X \n", PC);    
    printf("************************* \n");
}

//load 6502 binary into emu-RAM
int load(const char* file, address start_address)
{
    FILE *f;
    
    f = fopen(file, "r");	
    
    word k;
    address i = start_address;
    
    if (f == NULL)
    {
        printf("IO error: could not open file %s \n", file);
        return -1;
    }

    //load binary file into 6502 memory
    while(fread(&k, sizeof(word), 1, f) == 1) //this avoids reading the last line twice
    {
        mwr(k, i++);
    }
    fclose(f);	
    
    //dump loaded binary file
    printf("\nLoaded 6502 binary:");
    mdump(0, i);
    
    return 0;    
}

//(lo-addr x hi-addr) --> hilo addr
address lohi2addr(word lo, word hi)
{
    address a = hi;     //e.g. 00AB
    a = a << 8;         //e.g. AB00
    a = a | lo;         //e.g. AB00 | 00CD => ABCD
    
    return a;
}



