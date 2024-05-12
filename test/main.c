
#include <stdio.h>
#include <stdlib.h>
#include "../src/6502.h"
#include "../src/mem.h"
#include "../src/utils.h"
#include "../src/loader.h"

int main(int argc, char *argv[])
{	
    
    //exit if path to binary was not given
    if (argc < 2) 
    {
        printf("Input error: one argument expected: path to 6502-test-binary \n");
        return -1;
    }
        
    //init RAM
    TMemory mem = memInit();

    //init CPU and reset
    cpuInit();

    //connect memory to CPU
    cpuConnectMemory(mem);
    
    //load binary into RAM
    int status = loadProgramFromFile(mem, argv[1]);

    //exit if loading failed
    if (status != 0) return -1;
        
	//run
	while(1)
    {
        eCpuStepStatus status = cpuStep(); //step: fetch, decode, execute
        
        if (status != CPU_STEP_OK) 
        {
            printf("An error occurred during execution. Exiting now.\n");
            break;
        }
    }
    
}