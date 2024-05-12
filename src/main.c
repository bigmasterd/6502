/*********************************************
*** Load given 6502 binary and emulate it. ***
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include "6502.h"
#include "mem.h"
#include "utils.h"
#include "loader.h"


int main(int argc, char *argv[])
{	
    
    //exit if path to binary was not given
    if (argc < 2) 
    {
        printf("Input error: one argument expected: path to 6502-binary \n");
        return -1;
    }
        
    //init RAM
    TMemory mem = memInit();

    //init CPU
    T6502 cpu = cpuInit(mem);

    //load binary into RAM
    int status = loadProgramFromFile(mem, argv[1]);

    //exit if loading failed
    if (status != 0)
    {
        printf("Error: could not load program %s \n", argv[1]);
        return -2;
    }
        
	//run
	while(1)
    {
        eCpuStepStatus status = cpuStep(cpu); //step: fetch, decode, execute
        
        if (status != CPU_STEP_OK) 
        {
            printf("An error occurred during execution. Exiting now.\n");
            return -3;
        }
    }

    return 0;
}