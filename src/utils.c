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


void printRegs(void)
{
    printf("*** Register contents *** \n");
    printf("X:  %.2X \n", X);
    printf("Y:  %.2X \n", Y);
    printf("A:  %.2X \n", A);
    printf("P:  %.2X N=%d,V=%d,B=%d,D=%d,I=%d,Z=%d,C=%d\n", P, //(N V - B D I Z C)
                                                            ((P >> 7) & 0b00000001), 
                                                            ((P >> 6) & 0b00000001), 
                                                            ((P >> 4) & 0b00000001),
                                                            ((P >> 3) & 0b00000001),
                                                            ((P >> 2) & 0b00000001),
                                                            ((P >> 1) & 0b00000001),
                                                            ((P) & 0b00000001)); 
    printf("IR: %.2X \n", IR);
    printf("SP: %.2X \n", SP);    
    printf("PC: %.4X \n", PC);    
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


