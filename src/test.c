#include "test.h"
#include "6502.h"
#include <stdio.h>
#include <stdlib.h>


//6502 registers
extern word 	X;  //X indexing register
extern word 	Y;  //Y indexing register
extern word 	A;  //accumulator
extern word 	P;  //processor status word with the flags (N V - B D I Z C)
extern word 	IR; //instruction register, contains instruction to be decoded, i.e. IR == mrd(PC)
extern word  	SP; //6502's stack of 256 bytes range is located at 0100 to 01FF (hard wired). that is the 2nd frame in the RAM
extern address PC; //program counter, NOTE: PC contains always the instruction to be fetched next !!!


void preptest(word opcode)
{
    switch(opcode)
	{
        case TAX_IMPL:  
        {
            break;
        }
        
        case TXA_IMPL:  
        {
            X   =   0;
            Y   =   DEF_Y;
            A   =   DEF_A;
            P   =   DEF_P; 
            SP  =   DEF_SP;
            break;
        }
            
        default:
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;
            P   =   DEF_P; 
            SP  =   DEF_SP;    
            break;
        }
    }
}


void test(word opcode)
{
    switch(opcode)
	{
        //############################# TRANSFER INSTRUCTIONS #############################
	    case TAX_IMPL:  //X <- A, 1 byte long
        {
            check_reg(DEF_A, X, "X", "TAX_IMPL");
            check_reg(DEF_P, P, "P", "TAX_IMPL"); //P unchanged, since A was non negative and was not 0 (defaut case in preptest())
            break;
        }
        
        case TXA_IMPL:  //A <- X, 1 byte long
        {
            check_reg(0, A, "A", "TXA_IMPL");
            check_reg(0b00110010, P, "P", "TXA_IMPL"); //P changed, since X was 0
            break;
        }
            
        default:
        {
            break;
        }
    }
	
}

void check_reg(word exp_reg_val, word act_reg_value, char* reg_name, char* opcode_name)
{
    if (exp_reg_val != act_reg_value)
    {
        printf("%s: FAILED. Expected value in %s: 0x%x, but actual value: 0x%x.\n", opcode_name, reg_name, exp_reg_val, act_reg_value);
    }
    else
    {
        printf("%s: PASSED.\n", opcode_name);
    }
}