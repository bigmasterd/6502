#include <stdio.h>
#include <string.h>
#include "types.h"
#include "loader.h"

int loadProgram(TMemory mem, const word* program, uint32_t length)
{
    
    if (length > MEMSIZE)
    {
        printf("\nError: given program does not fit into 64K RAM.");
        return -1;        
    }
    
    //load binary file into 6502 memory
    address a = 0;
    for (uint32_t i = 0; i < length; i++)
    {
        memWrite(mem, program[i], a++);
    }
    
    //dump loaded binary file
    printf("\nLoaded 6502 binary:");
    memDump(mem, 0, a);
    
    return 0;
}


int loadProgramFromFile(TMemory mem, const char* file)
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
        memWrite(mem, w, a++);
    }
    fclose(f);	
    
    //dump loaded binary file
    printf("\nLoaded 6502 binary:");
    memDump(mem, 0, a);
    
    return 0;    
}