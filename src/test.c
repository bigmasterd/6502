#include "test.h"
#include "6502.h"
#include "mem.h"
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

#define DEF_X   0b00000001
#define DEF_Y   0b00000010
#define DEF_A   0b00000011
#define DEF_P   0b00110000 //normal default value: (N V - B D I Z C) // - always 1, B is 1 too because NES does not use decimal mode D at all
#define DEF_SP  0b00000100


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
            
        case TAY_IMPL:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   -42;
            P   =   DEF_P; 
            SP  =   DEF_SP;
            break;
        }
            
        case INY_IMPL:  
        {
            X   =   DEF_X;
            Y   =   0b11111111; //will turn into 0 after increment
            A   =   DEF_A;
            P   =   DEF_P; 
            SP  =   DEF_SP;
            break;
        }
            
        case CLC_IMPL:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;
            P   =   DEF_P | 0b00000001; //set carry to 1, to be deleted by CLC_IMPL 
            SP  =   DEF_SP;
            break;
        }
            
        case CLD_IMPL:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;
            P   =   DEF_P | 0b00001000; //set decimal flag to 1, to be deleted by CLD_IMPL 
            SP  =   DEF_SP;
            break;
        }
            
        case CLI_IMPL:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;
            P   =   DEF_P | 0b00000100; //set interrupt-disable flag to 1, to be deleted by CLI_IMPL 
            SP  =   DEF_SP;
            break;
        }
            
        case CLV_IMPL:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;
            P   =   DEF_P | 0b01000000; //set overflow flag to 1, to be deleted by CLV_IMPL 
            SP  =   DEF_SP;
            break;
        }
            
        case LDA_IMMD:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;  //will change to 0x23 after load
            P   =   DEF_P;  
            SP  =   DEF_SP;
            break;
        }
            
        case LDA_ZRP:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;  //will change to 0x77 after load
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x77, 0xab); //M[0x00ab] <- 0x77
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
	    case TAX_IMPL:  //X <- A, 1 byte long, affects N and Z
        {
            check_reg(DEF_A, X, "X", "TAX_IMPL");
            check_reg(DEF_P, P, "P", "TAX_IMPL"); //P unchanged, since A was non negative and was not 0 (defaut case in preptest())
            break;
        }
        
        case TXA_IMPL:  //A <- X, 1 byte long, affects N and Z
        {
            check_reg(0, A, "A", "TXA_IMPL");
            check_reg(0b00110010, P, "P", "TXA_IMPL"); //P changed, since X was 0
            break;
        }
            
        case TAY_IMPL:  //Y <- A, 1 byte long, affects N and Z
        {
            check_reg(-42, Y, "Y", "TAY_IMPL");
            check_reg(0b10110000, P, "P", "TAY_IMPL"); //P changed, since A was negative
            break;
        }
            
        case TYA_IMPL:  //A <- Y, 1 byte long, affects N and Z
        {
            check_reg(DEF_Y, A, "A", "TYA_IMPL");
            check_reg(DEF_P, P, "P", "TYA_IMPL"); 
            break;
        }
            
        case TSX_IMPL:  //X <- SP, 1 byte long, affects N and Z
        {
            check_reg(DEF_SP, X, "X", "TSX_IMPL");
            check_reg(DEF_P, P, "P", "TSX_IMPL"); 
            break;
        }
            
        case TXS_IMPL:  //SP <- X, 1 byte long, no flags
        {
            check_reg(DEF_X, SP, "SP", "TSX_IMPL");            
            break;
        }
            
        case INX_IMPL:  //X++, 1 byte long, affects N and Z
        {
            check_reg(DEF_X+1, X, "X", "INX_IMPL");            
            check_reg(DEF_P, P, "P", "INX_IMPL"); 
            break;
        }
            
        case INY_IMPL:  //Y++, 1 byte long, affects N and Z
        {
            check_reg( 0, Y, "Y", "INY_IMPL");            
            check_reg(DEF_P | 0b00000010, P, "P", "INY_IMPL"); //zero flag changed sinse 0b11111111 + 1 = 0 
            break;
        }
            
        case DEX_IMPL:  //X--, 1 byte long, affects N and Z
        {
            check_reg(DEF_X-1, X, "X", "DEX_IMPL");            
            check_reg(DEF_P | 0b00000010, P, "P", "DEX_IMPL"); //zero flag changed sinse 1 - 1 = 0 
            break;
        }
            
        case DEY_IMPL:  //Y--, 1 byte long, affects N and Z
        {
            check_reg(DEF_Y-1, X, "Y", "DEY_IMPL");            
            check_reg(DEF_P, P, "P", "DEY_IMPL");  
            break;
        }
            
        //############################# SET AND CLEAR INSTRUCTIONS #############################
            
        case SEC_IMPL:  //C <- 1, 1 byte long, affects C
        {
            check_reg(DEF_P | 0b00000001, P, "P", "SEC_IMPL"); //carry changed in P from 0 to 1  
            break;
        } 
            
        case SED_IMPL:  //D <- 1, 1 byte long, affects D
        {
            check_reg(DEF_P | 0b00001000, P, "P", "SED_IMPL"); //decimal flag changed in P from 0 to 1  
            break;
        } 
            
        case SEI_IMPL:  //I <- 1, 1 byte long, affects I
        {
            check_reg(DEF_P | 0b00000100, P, "P", "SEI_IMPL"); //interrupt flag changed in P from 0 to 1  
            break;
        }
         
        case CLC_IMPL:  //C <- 0, 1 byte long, affects C
        {
            check_reg(DEF_P, P, "P", "CLC_IMPL"); //carry flag set from 1 to 0   
            break;
        }
            
        case CLD_IMPL:  //D <- 0, 1 byte long, affects D
        {
            check_reg(DEF_P, P, "P", "CLD_IMPL"); //decimal flag set from 1 to 0   
            break;
        }
            
        case CLI_IMPL:  //I <- 0, 1 byte long, affects I
        {
            check_reg(DEF_P, P, "P", "CLI_IMPL"); //interrupt flag set from 1 to 0   
            break;
        }
            
        case CLV_IMPL:  //V <- 0, 1 byte long, affects V
        {
            check_reg(DEF_P, P, "P", "CLV_IMPL"); //overflow flag set from 1 to 0   
            break;
        }
            
        //############################# STORAGE INSTRUCTIONS #############################        
        case LDA_IMMD: //A <- M, 2 bytes long
        {
            check_reg(0x23, A, "A", "LDA_IMMD"); 
            break;  
        }         
            
        case LDA_ZRP: //A <- M[zrp], 2 bytes long
        {
            check_reg(0x77, A, "A", "LDA_ZRP"); 
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
        printf("%s: PASSED. Value in %s is just as expected: 0x%x.\n", opcode_name, reg_name, exp_reg_val);
    }
}