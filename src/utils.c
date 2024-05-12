#include <stdio.h>
#include <string.h>
#include <math.h>
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

//8bit integer to bit-extractor lookup table
word bitmasks[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};


void printRegs(T6502 cpu)
{
    printf("*** Register contents *** \n");
    printf("X:  0x%.2X (%s.%s) \n", cpu->X, int2bin[(cpu->X >> 4) & 0xF], int2bin[cpu->X & 0xF]);
    printf("Y:  0x%.2X (%s.%s) \n", cpu->Y, int2bin[(cpu->Y >> 4) & 0xF], int2bin[cpu->Y & 0xF]);    
    printf("A:  0x%.2X (%s.%s) \n", cpu->A, int2bin[(cpu->A >> 4) & 0xF], int2bin[cpu->A & 0xF]);
    printf("P:  0x%.2X N=%d,V=%d,B=%d,D=%d,I=%d,Z=%d,C=%d\n",  cpu->P, ((cpu->P >> 7) & 0x1), ((cpu->P >> 6) & 0x1), 
                ((cpu->P >> 4) & 0x1), ((cpu->P >> 3) & 0x1), ((cpu->P >> 2) & 0x1), ((cpu->P >> 1) & 0x1), ((cpu->P) & 0x1)); 
    printf("IR: 0x%.2X (%s.%s) \n", cpu->IR, int2bin[(cpu->IR >> 4) & 0xF], int2bin[cpu->IR & 0xF]);
    printf("SP: 0x%.2X (%s.%s) \n", cpu->SP, int2bin[(cpu->SP >> 4) & 0xF], int2bin[cpu->SP & 0xF]);
    printf("PC: 0x%.4X \n", cpu->PC);    
    printf("************************* \n");
}

void printExecInfo(word opcode)
{
    printf("Executing opcode 0x%.2X... \n", opcode);
}

#ifdef DELME

//load 6502 binary into emu-RAM
int load(const char* file)
{
    FILE *f;
    
    f = fopen(file, "r");	
        
    if (f == NULL)
    {
        printf("IO error: could not open file %s \n", file);
        return -1;
    }

    word w;
    address a = 0;
    
    //load binary file into 6502 memory
    while(fread(&w, sizeof(word), 1, f) == 1) //this avoids reading the last line twice
    {
        memWrite(w, a++);
    }
    fclose(f);	
    
    //dump loaded binary file
    printf("\nLoaded 6502 binary:");
    mdump(0, a);
    
    return 0;    
}
#endif

//(lo-addr x hi-addr) --> hilo addr
address lohi2addr(word lo, word hi)
{
    address a = hi;     //e.g. 00AB
    a = a << 8;         //e.g. AB00
    a = a | lo;         //e.g. AB00 | 00CD => ABCD
    
    return a;
}

//get bit from word
word getBit(word w, uint32_t bit_number)
{
    if (bit_number > 7)
    {
        printf("ERROR: can't get bit %u", bit_number);
        return 0xFF; //invalid
    }

    return (w & bitmasks[bit_number]) >> bit_number;
}

//set bit of word
void setBit(word* w, uint32_t bit_number)
{
    if (bit_number > 7)
    {
        printf("ERROR: can't set bit %u", bit_number);
        return;
    }
    
    *w |= bitmasks[bit_number];
}

//clear bit of word
void clearBit(word* w, uint32_t bit_number)
{
    if (bit_number > 7)
    {
        printf("ERROR: can't clear bit %u", bit_number);
        return;
    }
    
    *w &= ~bitmasks[bit_number];
}

