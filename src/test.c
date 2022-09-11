#include "test.h"
#include "6502.h"
#include "mem.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


//6502 registers
extern word 	X;  //X indexing register
extern word 	Y;  //Y indexing register
extern word 	A;  //accumulator
extern word 	P;  //processor status word with the flags (N V - B D I Z C)
extern word 	IR; //instruction register, contains instruction to be decoded, i.e. IR == mrd(PC)
extern word  	SP; //6502's stack of 256 bytes range is located at 0100 to 01FF (hard wired). that is the 2nd frame in the RAM
extern address PC;  //program counter, NOTE: PC contains always the instruction to be fetched next !!!

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
            
        case LDA_ZRPX:
        {
            X   =   0x7;
            Y   =   DEF_Y;
            A   =   DEF_A;  //will change to 0x78 after load
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x78, 0xab+X); //M[0x00ab+X] <- 0x78
            break;
        } 
            
        case LDA_ABS:
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   DEF_A;  //will change to 0x79 after load
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x79, 0xabcd); //M[0xabcd] <- 0x79
            break;
        } 
            
        case LDA_ABSX:
        {
            X   =   0x8;
            Y   =   DEF_Y;
            A   =   DEF_A;  //will change to 0x80 after load
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x80, 0xabcd+X); //M[0xabcd+X] <- 0x80
            break;
        } 
            
        case LDA_ABSY:
        {
            X   =   DEF_X;
            Y   =   0xaa;
            A   =   DEF_A;  //will change to 0x81 after load
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x81, 0xabcd+Y); //M[0xabcd+Y] <- 0x81
            break;
        } 
            
        case LDA_INDX:
        {
            //TODO
        }
            
        case LDA_INDY:
        {
            //TODO
        }
            
        case LDX_IMMD:  
        {
            X   =   DEF_X; //will change to 0x23 after load
            Y   =   DEF_Y;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            break;
        }
            
        case LDX_ZRP:  
        {
            X   =   DEF_X; //will change to 0x77 after load
            Y   =   DEF_Y;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x77, 0xab); //M[0x00ab] <- 0x77
            break;
        }
            
        case LDX_ZRPY:  
        {
            X   =   DEF_X; //will change to 0xcc after load
            Y   =   0xaa;  //offset
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0xcc, 0x42+Y); //M[0x0042+0x00aa] <- 0xcc
            break;
        }
            
        case LDX_ABS:
        {
            X   =   DEF_X; //will change to 0x12 after load
            Y   =   DEF_Y;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x12, 0xabcd); //M[0xabcd] <- 0x12
            break;
        }
            
            
        case LDX_ABSY:
        {
            X   =   DEF_X; //will change to 0xFA after load
            Y   =   0xbb;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0xFA, 0x1234+Y); //M[0xabcd+Y] <- 0xFA //negative
            break;
        } 
         
           
        case LDY_IMMD:  
        {
            X   =   DEF_X; 
            Y   =   DEF_Y; //will change to 0x23 after load
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            break;
        }
            
        case LDY_ZRP:  
        {
            X   =   DEF_X; 
            Y   =   DEF_Y; //will change to 0x77 after load
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x77, 0xab); //M[0x00ab] <- 0x77
            break;
        }
            
        case LDY_ZRPX:  
        {
            X   =   0xaa;  //offset
            Y   =   DEF_Y; //will change to 0xcc after load
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0xcc, 0x42+X); //M[0x0042+0x00aa] <- 0xcc
            break;
        }
            
        case LDY_ABS:
        {
            X   =   DEF_X; 
            Y   =   DEF_Y; //will change to 0x12 after load
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0x12, 0xabcd); //M[0xabcd] <- 0x12
            break;
        }
            
            
        case LDY_ABSX:
        {
            X   =   0xbb; 
            Y   =   DEF_Y; //will change to 0xFA after load
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;
            mwr(0xFA, 0x1234+X); //M[0xabcd+X] <- 0xFA //negative
            break;
        }
            
        case STA_ZRP:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   0x47;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STA_ZRPX:  
        {
            X   =   0x33;
            Y   =   DEF_Y;
            A   =   0x48;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STA_ABS:  
        {
            X   =   DEF_X;
            Y   =   DEF_Y;
            A   =   0x99;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STA_ABSX:  
        {
            X   =   0x33;
            Y   =   DEF_Y;
            A   =   0x99;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STA_ABSY:  
        {
            X   =   DEF_X;
            Y   =   0x44;
            A   =   0x88;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STA_INDX:  
        {
            //TODO;
        }
            
        case STA_INDY:  
        {
            //TODO;
        }
            
        case STX_ZRP:  
        {
            X   =   0xcc;
            Y   =   DEF_Y;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STX_ZRPY:  
        {
            X   =   0xcc;
            Y   =   0x01;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STX_ABS:  
        {
            X   =   0x33;
            Y   =   DEF_Y;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STY_ZRP:  
        {
            X   =   DEF_X;
            Y   =   0xcc;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STY_ZRPX:  
        {
            X   =   0x01; 
            Y   =   0xcc;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }
            
        case STY_ABS:  
        {
            X   =   DEF_X;
            Y   =   0x33;
            A   =   DEF_A;  
            P   =   DEF_P;  
            SP  =   DEF_SP;            
            break;
        }

        case INC_ZRP:
        {
            mwr(0x60, 0xAA);          
            break;
        }

        case INC_ZRPX:
        {
            X = 0x5;
            mwr(0xF0, 0xAA+X);          
            break;
        }

        case INC_ABS:
        {            
            mwr(0x12, 0x1234);          
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
            check_reg(DEF_Y-1, Y, "Y", "DEY_IMPL");            
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
            
        case LDA_ZRPX: //A <- M[zrp+X], 2 bytes long
        {
            check_reg(0x78, A, "A", "LDA_ZRPX"); 
            break;  
        }
            
        case LDA_ABS: //A <- M[abcd], 3 bytes long
        {
            check_reg(0x79, A, "A", "LDA_ABS"); 
            break;  
        }
            
        case LDA_ABSX: //A <- M[abcd+X], 3 bytes long
        {
            check_reg(0x80, A, "A", "LDA_ABSX"); 
            break;  
        }
            
        case LDA_ABSY: //A <- M[abcd+Y], 3 bytes long
        {
            check_reg(0x81, A, "A", "LDA_ABSY"); 
            break;  
        }
            
            
        case LDA_INDX:
        {
            //TODO
        }
            
        case LDA_INDY:
        {
            //TODO
        }


        case LDX_IMMD: //X <- M, 2 bytes long
        {
            check_reg(0x23, X, "X", "LDX_IMMD"); 
            break;  
        }      
            
        case LDX_ZRP: //X <- M[zrp], 2 bytes long
        {
            check_reg(0x77, X, "X", "LDX_ZRP"); 
            break;  
        }
            
        case LDX_ZRPY: //X <- M[zrp+Y], 2 bytes long
        {
            check_reg(0xcc, X, "X", "LDX_ZRPY"); 
            break;  
        }
            
        case LDX_ABS: //X <- M[abcd], 3 bytes long
        {
            check_reg(0x12, X, "X", "LDX_ABS"); 
            break;  
        }
            
        case LDX_ABSY: //X <- M[1234+Y], 3 bytes long
        {
            printRegs();
            check_reg(0xFA, X, "X", "LDX_ABSY"); 
            check_reg(0b10110000, P, "P", "LDX_ABSY"); //P changed, since value in mem was negative (0xFA)
            break;  
        }
             
        
        case LDY_IMMD: //Y <- M, 2 bytes long
        {
            printRegs();
            check_reg(0x23, Y, "Y", "LDY_IMMD"); 
            break;  
        }      
            
        case LDY_ZRP: //Y <- M[zrp], 2 bytes long
        {
            printRegs();
            check_reg(0x77, Y, "Y", "LDY_ZRP"); 
            break;  
        }
            
        case LDY_ZRPX: //Y <- M[zrp+Y], 2 bytes long
        {
            printRegs();
            check_reg(0xcc, Y, "Y", "LDY_ZRPX"); 
            break;  
        }
            
        case LDY_ABS: //Y <- M[abcd], 3 bytes long
        {
            printRegs();
            check_reg(0x12, Y, "Y", "LDY_ABS"); 
            break;  
        }
            
        case LDY_ABSX: //Y <- M[1234+Y], 3 bytes long
        {
            printRegs();
            check_reg(0xFA, Y, "Y", "LDY_ABSX"); 
            check_reg(0b10110000, P, "P", "LDY_ABSX"); //P changed, since value in mem was negative (0xFA)
            break;  
        }
            
        case STA_ZRP: //M[zrp] <--A, 2 bytes long
        {
            printRegs();
            check_mem(0xab, 0x47, "STA_ZRP"); //expecting value 0x47 in mem[0xab]
            break;  
        }
            
        case STA_ZRPX: //M[zrp+X] <--A, 2 bytes long
        {
            printRegs();
            check_mem(0xab+X, 0x48, "STA_ZRPX"); //expecting value 0x48 in mem[0xab+X]
            check_reg(DEF_P, P, "P", "STA_ZRPX"); //P unchanged!           
            break;  
        }
            
        case STA_ABS: //M[abcd] <--A, 3 bytes long
        {
            printRegs();
            check_mem(0x6666, 0x99, "STA_ABS"); //expecting value 0x99 in mem[0x6666]
            break;  
        }
            
        case STA_ABSX: //M[abcd+X] <--A, 3 bytes long
        {
            printRegs();
            check_mem(0x6666+X, 0x99, "STA_ABSX"); //expecting value 0x99 in mem[0x6666+X]
            break;  
        }
            
        case STA_ABSY: //M[abcd+Y] <--A, 3 bytes long
        {
            printRegs();
            check_mem(0x6666+Y, 0x88, "STA_ABSY"); //expecting value 0x88 in mem[0x6666+Y]
            break;  
        }
            
        case STA_INDX:  
        {
            //TODO;
        }
            
        case STA_INDY:  
        {
            //TODO;
        }
            
        case STX_ZRP: //M[zrp] <--X, 2 bytes long
        {
            printRegs();
            check_mem(0xab, 0xcc, "STX_ZRP"); //expecting value 0xcc in mem[0xab]
            break;  
        }
            
        case STX_ZRPY: //M[zrp+Y] <--X, 2 bytes long
        {
            printRegs();
            check_mem(0xab+Y, 0xcc, "STX_ZRPY"); //expecting value 0xcc in mem[0xab+Y]
            break;  
        }
            
        case STX_ABS: //M[abcd] <--X, 3 bytes long
        {
            printRegs();
            check_mem(0x1234, 0x33, "STX_ABS"); //expecting value 0x33 in mem[0x1234]
            break;  
        }
            
        case STY_ZRP: //M[zrp] <--Y, 2 bytes long
        {
            printRegs();
            check_mem(0xab, 0xcc, "STY_ZRP"); //expecting value 0xcc in mem[0xab]
            break;  
        }
            
        case STY_ZRPX: //M[zrp+X] <--Y, 2 bytes long
        {
            printRegs();
            check_mem(0xab+X, 0xcc, "STY_ZRPX"); //expecting value 0xcc in mem[0xab+X]
            break;  
        }
            
        case STY_ABS: //M[abcd] <--Y, 3 bytes long
        {
            printRegs();
            check_mem(0x1234, 0x33, "STY_ABS"); //expecting value 0x33 in mem[0x1234]
            break;  
        }

        
        case INC_ZRP: //M[zrp] <- M[zrp]+1, 3 bytes long
        {
            printRegs();
            check_mem(0xAA, 0x61, "INC_ZRP"); //expecting value 0x61 in mem[0xAA]
            break;  
        }

        case INC_ZRPX: //M[zrp+X] <- M[zrp]+1, 2 bytes long
        {
            printRegs();
            check_mem(0xAA+0x5, 0xf1, "INC_ZRPX"); //expecting value 0xf1 in mem[0xAF]
            check_reg(0b10110000, P, "P", "INC_ZRPX"); //P changed, since value in mem was negative (0xF1)
            break;  
        }
            
        case INC_ABS: //M[abcd] <- M[abcd]+1, 3 bytes long
        {
            printRegs();
            check_mem(0x1234, 0x13, "INC_ABS"); //expecting value 0x12+1 in mem[0x1234]
            break;  
        }
        
         
            
        default:
        {
            printf("There is no test for opcode %x", opcode);
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

void check_mem(address addr, word exp_mem_val, char* opcode_name)
{
    word act_mem_value = mrd(addr);
    
    if (exp_mem_val != act_mem_value)
    {
        printf("%s: FAILED. Expected value in mem[0x%x]: 0x%x, but actual value: 0x%x.\n", opcode_name, addr, exp_mem_val, act_mem_value);
    }
    else
    {
        printf("%s: PASSED. Value in mem[0x%x] is just as expected: 0x%x.\n", opcode_name, addr, exp_mem_val);
    }
}