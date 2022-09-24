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

word X_EXP;  //expected value in register X after test run
word Y_EXP;  //expected value in register Y after test run
word A_EXP;  //expected value in accumulator after test run
word P_EXP;  //expected value in status register after test run
word IR_EXP; //expected value in instruction register after test run
word SP_EXP; //expected value in stack pointer register after test run
word M_EXP;  //expected value in memory location

#define DEF_X   0b00000001
#define DEF_Y   0b00000010
#define DEF_A   0b00000011
#define DEF_P   0b00110000 //normal default value: (N V - B D I Z C) // - always 1, B is 1 too because NES does not use decimal mode D at all
#define DEF_SP  0b00000100

#define NO_TEST_PREP_IMPL_WARN(opcode) printf("NO TEST PREPARATION FOR OPCODE %X", opcode);
#define NO_TEST_IMPL_WARN(opcode) printf("NO TEST FOR OPCODE %X", opcode);


void preptest(word opcode)
{
    switch(opcode)
	{

        
        //############################# TRANSFER INSTRUCTIONS #############################        

        case TAX_IMPL:
        {
            NO_TEST_PREP_IMPL_WARN(TAX_IMPL);
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

        case TYA_IMPL:  
        {
            NO_TEST_PREP_IMPL_WARN(TYA_IMPL);
            break;
        }

        case TSX_IMPL:  
        {
            NO_TEST_PREP_IMPL_WARN(TSX_IMPL);
            break;
        }

        case TXS_IMPL:  
        {
            NO_TEST_PREP_IMPL_WARN(TXS_IMPL);
            break;
        }


        //############################# STORAGE INSTRUCTIONS #############################

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
            NO_TEST_PREP_IMPL_WARN(LDA_INDX);
            break;
        }
            
        case LDA_INDY:
        {
            NO_TEST_PREP_IMPL_WARN(LDA_INDY);
            break;
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
            NO_TEST_PREP_IMPL_WARN(STA_INDX);
            break;
        }
            
        case STA_INDY:  
        {
            NO_TEST_PREP_IMPL_WARN(STA_INDY);
            break;
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


        //############################# ARITHMETIC INSTRUCTIONS #############################

        case ADC_IMMD:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_IMMD);        
            break;
        }

        case ADC_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_ZRP);        
            break;
        }

        case ADC_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_ZRPX);        
            break;
        }

        case ADC_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_ABS);        
            break;
        }

        case ADC_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_ABSX);        
            break;
        }

        case ADC_ABSY:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_ABSY);        
            break;
        }

        case ADC_INDX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_INDX);        
            break;
        }

        case ADC_INDY:  
        {   
            NO_TEST_PREP_IMPL_WARN(ADC_INDY);        
            break;
        }

        case SBC_IMMD:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_IMMD);        
            break;
        }

        case SBC_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_ZRP);        
            break;
        }

        case SBC_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_ZRPX);        
            break;
        }

        case SBC_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_ABS);        
            break;
        }

        case SBC_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_ABSX);        
            break;
        }

        case SBC_ABSY:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_ABSY);        
            break;
        }

        case SBC_INDX:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_INDX);        
            break;
        }

        case SBC_INDY:  
        {   
            NO_TEST_PREP_IMPL_WARN(SBC_INDY);        
            break;
        }

//        
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


        case INC_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(INC_ABSX);        
            break;
        }

        case INX_IMPL:  
        {   
            NO_TEST_PREP_IMPL_WARN(INX_IMPL);        
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

        case DEC_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(DEC_ZRP);        
            break;
        }

        case DEC_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(DEC_ZRPX);        
            break;
        }

        case DEC_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(DEC_ABS);        
            break;
        }

        case DEC_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(DEC_ABSX);        
            break;
        }

        case DEX_IMPL:  
        {   
            NO_TEST_PREP_IMPL_WARN(DEX_IMPL);        
            break;
        }

        case DEY_IMPL:  
        {   
            NO_TEST_PREP_IMPL_WARN(DEY_IMPL);        
            break;
        }


        //############################# SHIFT & ROTATE INSTRUCTIONS #############################

        case ASL_ACCU:
        {   
            A = 0b10010011; //init A with test value
            P = 0; //init P
            A_EXP = 0b00100110; //bits shifted left, must change to 0b00100110                      
            P_EXP = 0b0000001; //carry must be set
            break;
        }

        case ASL_ZRP:  
        {   
            mwr(0b00010011, 0x0A); // init memory location that will be shifted
            M_EXP = 0b00100110; //expected value in memory after shift
            P = 0b1001001; //init P            
            P_EXP = 0b1001000; //carry must be NOT set, other bits must be unchanged            
            break;
        }

        case ASL_ZRPX:  
        {   
            X = 0x07;
            mwr(0b00010011, 0x0A+X); // init memory location that will be shifted
            M_EXP = 0b00100110; //expected value in memory after shift
            P = 0b1001001; //init P            
            P_EXP = 0b1001000; //carry must be NOT set, other bits must be unchanged            
            break;
        }

        case ASL_ABS:  
        {   
            mwr(0b11111110, 0x6789); // init memory location that will be shifted
            M_EXP = 0b11111100; //expected value in memory after shift
            P = 0b11111101; //init P            
            P_EXP = 0b11111101; //carry must be set, other bits must be unchanged            
            break;
        }

        case ASL_ABSX:  
        {   
            X = 0xEE;
            mwr(0b11111110, 0x6789+X); // init memory location that will be shifted
            M_EXP = 0b11111100; //expected value in memory after shift
            P = 0b11111101; //init P            
            P_EXP = 0b11111101; //carry must be set, other bits must be unchanged            
            break;
        }


        case LSR_ACCU:  
        {   
            NO_TEST_PREP_IMPL_WARN(LSR_ACCU);        
            break;
        }

        case LSR_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(LSR_ZRP);        
            break;
        }

        case LSR_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(LSR_ZRPX);        
            break;
        }

        case LSR_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(LSR_ABS);        
            break;
        }

        case LSR_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(LSR_ABSX);        
            break;
        }

        case ROL_ACCU:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROL_ACCU);        
            break;
        }

        case ROL_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROL_ZRP);        
            break;
        }

        case ROL_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROL_ZRPX);        
            break;
        }

        case ROL_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROL_ABS);        
            break;
        }

        case ROL_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROL_ABSX);        
            break;
        }

        case ROR_ACCU:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROR_ACCU);        
            break;
        }

        case ROR_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROR_ZRP);        
            break;
        }

        case ROR_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROR_ZRPX);        
            break;
        }

        case ROR_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROR_ABS);        
            break;
        }

        case ROR_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ROR_ABSX);        
            break;
        }


        //############################# LOGIC INSTRUCTIONS #############################

        case AND_IMMD:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_IMMD);
            break;
        }

        case AND_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_ZRP);
            break;
        }

        case AND_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_ZRPX);
            break;
        }

        case AND_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_ABS);
            break;
        }

        case AND_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_ABSX);
            break;
        }

        case AND_ABSY:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_ABSY);
            break;
        }

        case AND_INDX:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_INDX);
            break;
        }

        case AND_INDY:  
        {   
            NO_TEST_PREP_IMPL_WARN(AND_INDY);
            break;
        }

        case ORA_IMMD:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_IMMD);
            break;
        }

        case ORA_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_ZRP);
            break;
        }

        case ORA_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_ZRPX);
            break;
        }

        case ORA_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_ABS);
            break;
        }

        case ORA_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_ABSX);
            break;
        }

        case ORA_ABSY:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_ABSY);
            break;
        }

        case ORA_INDX:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_INDX);
            break;
        }

        case ORA_INDY:  
        {   
            NO_TEST_PREP_IMPL_WARN(ORA_INDY);
            break;
        }

        case EOR_IMMD:  
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_IMMD);
            break;
        }

        case EOR_ZRP:  
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_ZRP);
            break;
        }

        case EOR_ZRPX:  
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_ZRPX);
            break;
        }

        case EOR_ABS:  
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_ABS);
            break;
        }

        case EOR_ABSX:  
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_ABSX);
            break;
        }

        case EOR_ABSY:  
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_ABSY);
            break;
        }

        case EOR_INDX:  
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_INDX);
            break;
        }

        case EOR_INDY:
        {   
            NO_TEST_PREP_IMPL_WARN(EOR_INDY);
            break;
        }


        //############################# COMPARE AND TEST BIT INSTRUCTIONS #############################

        case CMP_IMMD:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_IMMD);
            break;
        }

        case CMP_ZRP:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_ZRP);
            break;
        }

        case CMP_ZRPX:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_ZRPX);
            break;
        }

        case CMP_ABS:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_ABS);
            break;
        }

        case CMP_ABSX:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_ABSX);
            break;
        }

        case CMP_ABSY:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_ABSY);
            break;
        }

        case CMP_INDX:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_INDX);
            break;
        }

        case CMP_INDY:
        {   
            NO_TEST_PREP_IMPL_WARN(CMP_INDY);
            break;
        }

        case CPX_IMMD:
        {   
            NO_TEST_PREP_IMPL_WARN(CPX_IMMD);
            break;
        }

        case CPX_ZRP:
        {   
            NO_TEST_PREP_IMPL_WARN(CPX_ZRP);
            break;
        }

        case CPX_ABS:
        {   
            NO_TEST_PREP_IMPL_WARN(CPX_ABS);
            break;
        }

        case CPY_IMMD:
        {   
            NO_TEST_PREP_IMPL_WARN(CPY_IMMD);
            break;
        }

        case CPY_ZRP:
        {   
            NO_TEST_PREP_IMPL_WARN(CPY_ZRP);
            break;
        }

        case CPY_ABS:
        {   
            NO_TEST_PREP_IMPL_WARN(CPY_ABS);
            break;
        }

        case BIT_ZRP:
        {   
            NO_TEST_PREP_IMPL_WARN(BIT_ZRP);
            break;
        }

        case BIT_ABS:
        {   
            NO_TEST_PREP_IMPL_WARN(BIT_ABS);
            break;
        }


        //############################# SET AND CLEAR INSTRUCTIONS #############################

        case SEC_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(SEC_IMPL);
            break;
        }

        case SED_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(SED_IMPL);
            break;
        }

        case SEI_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(SEI_IMPL);
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


        //############################# JUMP AND SUBROUTINE INSTRUCTIONS #############################

        case JMP_ABS:
        {   
            NO_TEST_PREP_IMPL_WARN(JMP_ABS);
            break;
        }

        case JMP_IND:
        {   
            NO_TEST_PREP_IMPL_WARN(JMP_IND);
            break;
        }

        case JSR_ABS:
        {   
            NO_TEST_PREP_IMPL_WARN(JSR_ABS);
            break;
        }

        case RTS_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(RTS_IMPL);
            break;
        }

        case RTI_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(RTI_IMPL);
            break;
        }


        //############################# BRANCH INSTRUCTIONS #############################

        case BCC_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BCC_REL);
            break;
        }

        case BCS_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BCS_REL);
            break;
        }

        case BEQ_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BEQ_REL);
            break;
        }

        case BMI_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BMI_REL);
            break;
        }

        case BNE_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BNE_REL);
            break;
        }

        case BPL_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BPL_REL);
            break;
        }

        case BVC_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BVC_REL);
            break;
        }
        
        case BVS_REL:
        {   
            NO_TEST_PREP_IMPL_WARN(BVS_REL);
            break;
        }


        //############################# STACK INSTRUCTIONS #############################
                
        case PHA_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(PHA_IMPL);
            break;
        }
        
        case PLA_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(PLA_IMPL);
            break;
        }

        case PHP_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(PHP_IMPL);
            break;
        }
        
        case PLP_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(PLP_IMPL);
            break;
        }
    

        //############################# MISC INSTRUCTIONS #############################

        case NOP_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(NOP_IMPL);
            break;
        }
        
        case BRK_IMPL:
        {   
            NO_TEST_PREP_IMPL_WARN(BRK_IMPL);
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


        //############################# ARITHMETIC INSTRUCTIONS #############################

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


        //############################# SHIFT & ROTATE INSTRUCTIONS #############################

        case ASL_ACCU: //A <- [A << 1], original bit #7 is stored to carry flag
        {
            printRegs();
            check_reg(A_EXP, A, "A", "ASL_ACCU");
            check_reg(P_EXP, P, "P", "ASL_ACCU");
            break;  
        }

        case ASL_ZRP: //M[zrp] <- [M[zrp] << 1], original bit #7 is stored to carry flag
        {
            printRegs();
            check_mem(0x0A, M_EXP, "ASL_ZRP"); //expecting value M_EXP in mem[0x0A]
            check_reg(P_EXP, P, "P", "ASL_ZRP");
            break;  
        }        

        case ASL_ZRPX: //M[zrp+X] <- [M[zrp+X] << 1], original bit #7 is stored to carry flag
        {
            printRegs();
            check_mem(0x0A+X, M_EXP, "ASL_ZRP"); //expecting value M_EXP in mem[0x0A+X]
            check_reg(P_EXP, P, "P", "ASL_ZRP");
            break;  
        }        

        case ASL_ABS: //M[abcd] <- [M[abcd] << 1], original bit #7 is stored to carry flag
        {
            printRegs();
            check_mem(0x6789, M_EXP, "ASL_ABS"); //expecting value M_EXP in mem[0xabcd]
            check_reg(P_EXP, P, "P", "ASL_ABS");
            break;  
        }        

        case ASL_ABSX: //M[abcd+X] <- [M[abcd+X] << 1], original bit #7 is stored to carry flag
        {
            printRegs();
            check_mem(0x6789+X, M_EXP, "ASL_ABSX"); //expecting value M_EXP in mem[0xabcd+X]
            check_reg(P_EXP, P, "P", "ASL_ABSX");
            break;  
        }


        //############################# LOGIC INSTRUCTIONS #############################

        //############################# COMPARE AND TEST BIT INSTRUCTIONS #############################


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
        

        //############################# JUMP AND SUBROUTINE INSTRUCTIONS #############################

            
        //############################# BRANCH INSTRUCTIONS #############################    


        //############################# STACK INSTRUCTIONS #############################

        //############################# MISC INSTRUCTIONS #############################

        
        
        
         
            
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