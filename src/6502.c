//TODO: test already implemented OPCODES first !!!!

//TODO:
//  -a
//  -b
//    - implement adplusb.asm

/* NOTES:
*
* Little Endian: address ABCD is stored as:
*   address n, address n+1 
*   CD         AB           i.e. "little significance first"
*
*/

#include <stdio.h>
#include <stdlib.h>
#include "6502.h"
#include "mem.h"
#include "utils.h"
#include "test.h"

#define START_ADDRESS 0x0

//enable/disable test mode here
#define TEST_MODE

#ifdef TEST_MODE 
    #define PREPTEST(opcode) preptest(opcode)
    #define TEST(opcode) test(opcode)
#else 
    #define PREPTEST(opcode)    //expand to nothing
    #define TEST(opcode)        //expand to nothing
#endif

//6502 registers
word 	X;  //X indexing register
word 	Y;  //Y indexing register
word 	A;  //accumulator
word 	P;  //processor status word with the flags (N V - B D I Z C)
word 	IR; //instruction register, contains instruction to be decoded, i.e. IR == mrd(PC)
word  	SP; //6502's stack of 256 bytes range is located at 0100 to 01FF (hard wired). that is the 2nd frame in the RAM
address PC; //program counter, NOTE: PC contains always the instruction to be fetched next !!!


void reset()
{
    X   =   0x0;
    Y   =   0x0;
    A   =   0x0;
    P   =   0x30; //00110000 = (N V - B D I Z C) // - always 1, B is 1 too because NES does not use decimal mode D at all
    IR  =   0x0;    
    SP  =   0x0;     
	PC  =   START_ADDRESS;
}


//set N in P = (N V - B D I Z C) if value in reg is negative
void setN(word reg)
{
    word tmp = reg >> 7; //shift register value 7 bits to right => get only 1st bit, i.e. 0b00000001
    //alternatively: if (((sword) reg) < 0)         (not tested though)
    
    if (tmp == 1) 
    {
        P = P | 0b10000000; //N = 1 => register value is negative
    }
    else 
    {
        P = P & 0b01111111; //N = 0 => register value is non negative        
    }
}

//set V in P = (N V - B D I Z C) 
void setV(int flag)
{
    if (flag == 1) P = P | 0b01000000; //V = 1 => overflow occured
    else P = P & 0b10111111; 
}

//set B in P = (N V - B D I Z C) 
void setB(int flag)
{
    if (flag == 1) P = P | 0b00010000; 
    else P = P & 0b11101111; 
}

//set D in P = (N V - B D I Z C) 
void setD(int flag)
{
    if (flag == 1) P = P | 0b00001000; 
    else P = P & 0b11110111; 
}

//set I in P = (N V - B D I Z C) 
void setI(int flag)
{
    if (flag == 1) P = P | 0b00000100; 
    else P = P & 0b11111011; 
}

//set Z in P = (N V - B D I Z C)
void setZ(word reg)
{
    if (reg == 0) P = P | 0b00000010; //register is zero => set Z to 1
    else P = P & 0b11111101; //register is non zero => set Z to 0
}

//set C in P = (N V - B D I Z C) 
void setC(int flag)
{
    if (flag == 1) P = P | 0b00000001; 
    else P = P & 0b11111110; 
}

//get N from P = (N V - B D I Z C)
int getN(void)
{
    //move N bit to the right most location and clear all bits bevore N => result is 0 or 1
    int n = (P >> 7) & 0b00000001; 
    return n;
}

//get V from P = (N V - B D I Z C)
int getV(void)
{
    //move V bit to the right most location and clear all bits bevore V => result is 0 or 1
    int v = (P >> 6) & 0b00000001; 
    return v;
}

//get B from P = (N V - B D I Z C)
int getB(void)
{
    //move B bit to the right most location and clear all bits bevore B => result is 0 or 1
    int b = (P >> 4) & 0b00000001; 
    return b;
}

//get D from P = (N V - B D I Z C)
int getD(void)
{
    //move D bit to the right most location and clear all bits bevore D => result is 0 or 1
    int d = (P >> 3) & 0b00000001; 
    return d;
}

//get I from P = (N V - B D I Z C)
int getI(void)
{
    //move I bit to the right most location and clear all bits bevore I => result is 0 or 1
    int i = (P >> 2) & 0b00000001; 
    return i;
}

//get Z from P = (N V - B D I Z C) 
int getZ(void)
{
    //move Z bit to the right most location and clear all bits bevore Z => result is 0 or 1
    int z = (P >> 1) & 0b00000001; 
    return z;
}

//get C from P = (N V - B D I Z C)
int getC(void)
{
    //clear all bits bevore C => result is 0 or 1
    int c = P & 0b00000001; 
    return c;
}

//checks whether specified word is negative or not
int isN(word w)
{
    //move bit 7 to the right most location and clear all bits 0 - 6 => result is 0 or 1
    int n = (w >> 7) & 0b00000001; 
    return n;
}

address getZrpAddr(void)
{
    address a = mrd(PC+1);              //get address from zeropage
    return a;
}

//The address calculation wraps around if the sum of the base address and the register exceed $FF (e.g. $80 + $FF => $7F) and not $017F.
address getZrpXAddr(void)
{
    address a = mrd(PC+1);              //get address from zeropage    
    a = (a + X) & 0x00FF;               //address must be within zero page => wrap around if a+X > 8bit address (i.e. clear MSB);            
    return a; 
}

address getZrpYAddr(void)
{
    address a = mrd(PC+1);              //get address from zeropage    
    a = (a + Y) & 0x00FF;               //address must be within zero page => wrap around if a+Y > 8bit address;            
    return a; 
}

//operand is absolute address which is stored in little endian format, e.g. CDAB
//the actual mem address we want to access is ABCD though, hence convert little endian CDAB to address ABCD, then we can do mem[ABCD]
address getAbsAddr(void)
{
    word lo = mrd(PC+1);                //get least significat byte of operand address (little endian)
    word hi = mrd(PC+2);                //get most significat byte of operand address (little endian)
    return lohi2addr(lo,hi);            //convert lo byte and hi byte to a 16bit address    
}

//NOTE: no wrapping here if final address > 16bit, 6502 programmer must care by himself!!!
address getAbsXAddr(void)
{
    word lo = mrd(PC+1);                //get least significat byte of operand address (little endian)
    word hi = mrd(PC+2);                //get most significat byte of operand address (little endian)
    return lohi2addr(lo,hi) + X;        //convert lo byte and hi byte to a 16bit address and add X 
}

//NOTE: no wrapping here if final address > 16bit, 6502 programmer must care by himself!!!
address getAbsYAddr(void)
{
    word lo = mrd(PC+1);                //get least significat byte of operand address (little endian)
    word hi = mrd(PC+2);                //get most significat byte of operand address (little endian)
    return lohi2addr(lo,hi) + Y;        //convert lo byte and hi byte to a 16bit address and add Y 
}



word getImdOp(void)
{
    word operand = mrd(PC+1);
    return operand;
}


word getZrpOp(void)
{
    address a = getZrpAddr();           //get operand address within zero page, i.e. 8 bit address
    word operand = mrd(a);              //get operand
    return operand;
}


word getZrpXOp(void)
{
    address a = getZrpXAddr();          //get operand address within zero page + X, i.e. 8 bit address
    word operand = mrd(a);              //get operand
    return operand;
}

word getZrpYOp(void)
{
    address a = getZrpYAddr();          //get operand address within zero page + Y, i.e. 8 bit address
    word operand = mrd(a);              //get operand
    return operand;
}

word getAbsOp(void)
{
    address a = getAbsAddr();           //get absolute address, i.e. PC+1 concat PC+2
    word operand = mrd(a);              //get operand
    return operand;
}

word getAbsXOp(void)
{
    address a = getAbsXAddr();          //get absolute address + X, i.e. (PC+1 concat PC+2) + X
    word operand = mrd(a);              //get operand
    return operand;
}

word getAbsYOp(void)
{
    address a = getAbsYAddr();          //get absolute address + Y, i.e. (PC+1 concat PC+2) + Y
    word operand = mrd(a);              //get operand
    return operand;
}

//The value in X is added to the specified zero page address for a sum address. 
//The little-endian address stored at the two-byte pair of sum address (LSB) and sum address plus one (MSB) is loaded 
//and the value at that address is used to perform the computation.
//Example: The value $02 in X is added to $15 for a sum of $17. 
//The address $D010 at addresses $0017 and $0018 will be where the value $0F in the accumulator is stored.
//STA ($15,X)
//...
//Another descripion: This mode is only used with the X register. 
//Consider a situation where the instruction is LDA ($20,X), X contains $04, and memory at $/24 contains 0024: 74 20, 
//First, X is added to $20 to get $24. 
//The target address will be fetched from $24 resulting in a target address of $2074. 
//Register A will be loaded with the contents of memory at $2074.
//If X + the immediate byte will wrap around to a zero-page address. 
//So you could code that like targetAddress = X + opcode[1]) & 0xFF .
//IN SHORT: get a 16bit pointer from Zeropage and use it to access operand somewhere else in Zeropage
word getIndXOp(void)
{
    //TODO: check and test this all, 
    //TODO: check idea behind wrap around (0xFF)
    word zrp_addr = mrd(PC+1);                                      //get address, it's an 8bit zero page address
    word operand_addr_lo = (zrp_addr + (sword) X) & 0xFF;           //get lo part of the operand address
    word operand_addr_hi = ((zrp_addr + (sword) X) + 1) & 0xFF;     //get hi part of the operand address
    address a = lohi2addr(operand_addr_lo, operand_addr_hi);        //convert lo byte and hi byte to a 16bit address
    word operand = mrd(a);                                          //finally, get operand
    return operand;
}

//This mode is only used with the Y register. It differs in the order that Y is applied to the indirectly fetched address. 
//An example instruction that uses indirect index addressing is LDA ($86),Y . 
//To calculate the target address, the CPU will first fetch the address stored at zero page location $86. 
//That address will be added to register Y to get the final target address. 
//For LDA ($86),Y, if the address stored at $86 is $4028 (memory is 0086: 28 40, remember little endian) and register Y contains $10, 
//then the final target address would be $4038. Register A will be loaded with the contents of memory at $4038.
//Indirect Indexed instructions are 2 bytes - the second byte is the zero-page address - $20 in the example. 
//(So the fetched address has to be stored in the zero page.)
//IMPORTANT: While indexed indirect addressing will only generate a zero-page address, 
//this mode's target address is not wrapped - it can be anywhere in the 16-bit address space.
//IN SHORT: get a 16bit pointer from Zeropage and use it to access operand somewhere else in 16bit address range
address getIndYOp(void)
{
    word zrp_addr = mrd(PC+1);                                  //get address, it's an 8bit zero page address   
    word operand_addr_lo = zrp_addr;                            //get lo part of the operand address
    word operand_addr_hi = zrp_addr + 1;                        //get hi part of the operand address
    address a = lohi2addr(operand_addr_lo, operand_addr_hi);    //convert lo byte and hi byte to a 16bit address
    a = a + (sword) Y;                                          //add Y to calculated address
    word operand = mrd(a);                                      //finally, get operand
    return operand;
}


int main(int argc, char *argv[])
{	
    
    //exit if path to binary was not given
    if (argc < 2) 
    {
        printf("Input error: one argument expected: path to 6502-binary \n");
        return -1;
    }
    
    //load binary into emu-RAM
    int load_status = load(argv[1], START_ADDRESS);
    
    //exit if loading failed
    if (load_status != 0) return -1;
    
    //init registers
    reset();
	
    //initially, processor is running
	int running = 1;
    
	//here we go: fetch, decode, execute
	while(running)
	{
        //fetch 
		IR = mrd(PC);
        
        //decode, then execute
		switch(IR)
		{
            //############################# TRANSFER INSTRUCTIONS #############################
			case TAX_IMPL:  //X <- A, 1 byte long
            {
                PREPTEST(TAX_IMPL);
            
                PC++;                           //target next opcode
                tax();                          //execute opcode      
                        
                TEST(TAX_IMPL);            
                break;
            }
            
            case TXA_IMPL:  //A <- X, 1 byte long
            {
                PREPTEST(TXA_IMPL);
            
                PC++;                           //target next opcode
                txa();                          //execute opcode      
            
                TEST(TXA_IMPL);
                break;  
            }
            
            case TAY_IMPL:  //Y <- A, 1 byte long
            {
                PREPTEST(TAY_IMPL);
            
                PC++;                           //target next opcode
                tay();                          //execute opcode
            
                TEST(TAY_IMPL);
                break;  
            }
            
            case TYA_IMPL:  //A <- Y, 1 byte long
            {
                PREPTEST(TYA_IMPL);
            
                PC++;                           //target next opcode
                tya();                          //execute opcode      
            
                TEST(TYA_IMPL);            
                break;  
            }
                
            case TSX_IMPL:  //X <- SP, 1 byte long
            {
                PREPTEST(TSX_IMPL);
            
                PC++;                           //target next opcode
                tsx();                          //execute opcode      
            
                TEST(TSX_IMPL);
                break;  
            }
             
            case TXS_IMPL:  //SP <- X, 1 byte long
            {
                PREPTEST(TXS_IMPL);
            
                PC++;                           //target next opcode
                txs();                          //execute opcode      
            
                TEST(TXS_IMPL);            
                break;  
            }
                
            
            //############################# STORAGE INSTRUCTIONS #############################
            //************ LDA: A <- M *************
            case LDA_IMMD: //A <- M, 2 bytes long
            {
                PREPTEST(LDA_IMMD);
            
                word operand = getImdOp();      //target operand 
                PC += 2;                        //target next opcode 
                lda(operand);                   //execute opcode
            
                TEST(LDA_IMMD);
                break;  
            }                
            case LDA_ZRP: //A <- M from zeropage, 2 bytes long
            {
                PREPTEST(LDA_ZRP);
            
                word operand = getZrpOp();      //get operand from zeropage
                PC += 2;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ZRP);
                break;  
            }         
            //TODO/TOCHECK/LATER
            case LDA_ZRPX: //A <- M from zeropage+X, 2 bytes long
            {
                PREPTEST(LDA_ZRPX);
            
                word operand = getZrpXOp();     //get operand from zeropage 
                PC += 2;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ZRPX);
                break;  
            }
            case LDA_ABS: //A <- M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(LDA_ABS);
            
                word operand = getAbsOp();      //get operand from absolute address  
                PC += 3;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ABS);
                break;                          
            }
            
            case LDA_ABSX: //A <- M from [[PChi,PClo]+X], 3 bytes long
            {
                PREPTEST(LDA_ABSX);
                
                word operand = getAbsXOp();     //get operand from absolute address + X
                PC += 3;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ABSX);
                break;                          
            }
            //TODO/TOCHECK/LATER
            case LDA_ABSY: //A <- M from [[PChi,PClo]+Y], 3 bytes long
            {
                PREPTEST(LDA_ABSY);
            
                word operand = getAbsYOp();     //get operand from absolute address + Y
                PC += 3;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ABSY);
                break;                          
            }
            //TODO/TOCHECK/LATER
            case LDA_INDX: //A <- M [TODO], 2 bytes long
            {
                word operand = getIndXOp();
                PC += 2;
                lda(operand);
                break;
            }
            //TODO/TOCHECK/LATER
            case LDA_INDY: //A <- M [TODO], 2 bytes long
            {
                word operand = getIndYOp();
                PC += 2;
                lda(operand);
                break;
            }
            
            //************ LDA: X <- M *************
            case LDX_IMMD: //X <- M, e.g. LDX #$FF ; load X with $FF, 2 bytes long
            {
                PREPTEST(LDX_IMMD);
            
                word operand = getImdOp();      //target operand 
                PC += 2;                        //target next opcode
                ldx(operand); 
            
                TEST(LDX_IMMD);
                break;  
            }
                
            case LDX_ZRP: //X <- M from zeropage, 2 bytes long
            {
                PREPTEST(LDX_ZRP);
            
                word operand = getZrpOp();     //get operand from zeropage 
                PC += 2;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ZRP);
                break;
            }
            
                
            case LDX_ZRPY: //X <- M from zeropage+Y, 2 bytes long
            {
                PREPTEST(LDX_ZRPY);
            
                word operand = getZrpYOp();     //get operand from zeropage 
                PC += 2;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ZRPY);
                break; 
            }
            case LDX_ABS: //X <- M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(LDX_ABS);
            
                word operand = getAbsOp();      //get operand from absolute address  
                PC += 3;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ABS);
                break;         
            }
            
            case LDX_ABSY: //X <- M from [[PChi,PClo]+Y], 3 bytes long
            {
                PREPTEST(LDX_ABSY);
            
                word operand = getAbsYOp();     //get operand from absolute address + Y
                PC += 3;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ABSY);
                break; 
            }
             
            //************ LDY: Y <- M *************
            case LDY_IMMD: //X <- M, e.g. LDX #$FF ; load X with $FF, 2 bytes long
            {
                PREPTEST(LDY_IMMD);
            
                word operand = getImdOp();      //target operand 
                PC += 2;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_IMMD);
                break;  
            }
            case LDY_ZRP: //X <- M from zeropage, 2 bytes long
            {
                PREPTEST(LDY_ZRP);
            
                word operand = getZrpOp();     //get operand from zeropage 
                PC += 2;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ZRP);
                break;
            }
            
            case LDY_ZRPX: //X <- M from zeropage+X, 2 bytes long
            {
                PREPTEST(LDY_ZRPX);
            
                word operand = getZrpXOp();     //get operand from zeropage 
                PC += 2;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ZRPX);
                break; 
            }
            
            case LDY_ABS: //X <- M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(LDY_ABS);
            
                word operand = getAbsOp();      //get operand from absolute address  
                PC += 3;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ABS);
                break;         
            }
            
            case LDY_ABSX: //X <- M from [[PChi,PClo]+X], 3 bytes long
            {
                PREPTEST(LDY_ABSX);
            
                word operand = getAbsXOp();     //get operand from absolute address + X
                PC += 3;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ABSX);
                break; 
            }
                
                
            //************ STA: A -> M *************
            case STA_ZRP: //A -> M from zeropage, 2 bytes long
            {               
                PREPTEST(STA_ZRP);
            
                address a = getZrpAddr();       //get address from zeropage
                PC += 2;                        //target next opcode
                sta(a);                         //execute opcode
            
                TEST(STA_ZRP);
                break;  
            }    
            
            case STA_ZRPX: //A -> M from zeropage+X, 2 bytes long
            {
                PREPTEST(STA_ZRPX);
            
                address a = getZrpXAddr();      //get address from zeropage + X
                PC += 2;                        //target next opcode
                sta(a);                         //execute opcode
            
                TEST(STA_ZRPX);
                break;  
            }                   
            case STA_ABS: //A -> M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(STA_ABS);
            
                address a = getAbsAddr();       //get absolute address
                PC += 3;                        //target next opcode
                sta(a);                         //execute opcode
            
                TEST(STA_ABS);
                break;                          
            }
            
            case STA_ABSX: //A -> M from [PChi,PClo] + X, 3 bytes long
            {
                PREPTEST(STA_ABSX);
            
                address a = getAbsXAddr();      //get absolute address + X
                PC += 3;                        //target next opcode
                sta(a);                         //execute opcode
                
                TEST(STA_ABSX);
                break;                          
            }
            
            case STA_ABSY: //A -> M from [PChi,PClo] + Y, 3 bytes long
            {
                PREPTEST(STA_ABSY);
            
                address a = getAbsYAddr();      //get absolute address + Y
                PC += 3;                        //target next opcode
                sta(a);                         //execute opcode
            
                TEST(STA_ABSY);
                break;                          
            }
                
            //TODO/TOCHECK/LATER 
            /*
            //STA ($20,X)	Stores the content of the Accumulator to the address obtained from the address calculated from "$20 adding content of Index Register X"
            case LDA_INDXX_TEMPLATE: //A -> M [TODO], 2 bytes long
            {
                word operand = getIndXOp();
                PC += 2;
                lda(operand);
                break;
            }
            case LDA_INDYX_TEMPLATE: //A <- M [TODO], 2 bytes long
            {
                word operand = getIndYOp();
                PC += 2;
                lda(operand);
                break;
            }
            */  
            
                
            //************ STX: X -> M *************
            case STX_ZRP: //X -> M from zeropage, 2 bytes long
            {               
                PREPTEST(STX_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage
                PC += 2;                        //target next opcode
                stx(a);                         //execute opcode
                
                TEST(STX_ZRP);
                break;  
            } 
                
            //************ STX: X -> M *************
            case STX_ZRPY: //X -> M from zeropage+Y, 2 bytes long
            {               
                PREPTEST(STX_ZRPY);
                
                address a = getZrpYAddr();      //get address from zeropage + Y
                PC += 2;                        //target next opcode
                stx(a);                         //execute opcode
                
                TEST(STX_ZRPY);
                break;  
            }
                
                
            case STX_ABS: //X -> M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(STX_ABS);
                
                address a = getAbsAddr();       //get absolute address
                PC += 3;                        //target next opcode
                stx(a);                         //execute opcode
                
                TEST(STX_ABS);
                break;                          
            }
                
              
            //************ STY: Y -> M *************                
            case STY_ZRP: //Y -> M from zeropage, 2 bytes long
            {               
                PREPTEST(STY_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage
                PC += 2;                        //target next opcode
                sty(a);                         //execute opcode
                
                TEST(STY_ZRP);
                break;  
            } 
                
            //************ STY: Y -> M *************
            case STY_ZRPX: //Y -> M from zeropage+X, 2 bytes long
            {               
                PREPTEST(STY_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage + X
                PC += 2;                        //target next opcode
                sty(a);                         //execute opcode
                
                TEST(STY_ZRPX);
                break;  
            }
                
                
            case STY_ABS: //Y -> M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(STY_ABS);
                
                address a = getAbsAddr();       //get absolute address
                PC += 3;                        //target next opcode
                sty(a);                         //execute opcode
                
                TEST(STY_ABS);
                break;                          
            }

            
                
                
                
            //############################# ARITHMETIC INSTRUCTIONS #############################
            //ADC:  Add Memory to Accumulator with Carry: A <- A + M + C
            case ADC_IMMD: //2 bytes long
            {
                word operand = getImdOp();      //target operand 
                PC += 2;                        //target next opcode
                adc(operand);                   //execute opcode
                break;                   
            }
            case ADC_ZRP: //2 bytes long
            {
                word operand = getZrpOp();      //get operand from zeropage
                PC += 2;                        //target next opcode
                adc(operand);                   //execute opcode
                break;                   
            }
            case ADC_ABS: //2 bytes long
            {
                word operand = getAbsOp();      //get operand from absolute address
                PC += 2;                        //target next opcode
                adc(operand);                   //execute opcode
                break;                   
            }
                
            //SBC: Subtract Memory from Accumulator with Borrow: A - M - !C -> A
            case SBC_IMMD: //2 bytes long
            {
                word operand = getImdOp();      //target operand 
                PC += 2;                        //target next opcode
                sbc(operand);                   //execute opcode
                break;
            }
            
            
            
            case INX_IMPL: 
            {
                PREPTEST(INX_IMPL);
            
                PC++;                //target next opcode
                inx();               //X <- X + 1, 1 byte long
            
                TEST(INX_IMPL);
                break;        
            }
                
            case INY_IMPL: 
            {
                PREPTEST(INY_IMPL);
            
                PC++;                //target next opcode
                iny();               //Y <- Y + 1, 1 byte long
            
                TEST(INY_IMPL);
                break;        
            }
                
            case DEX_IMPL: 
            {
                PREPTEST(DEX_IMPL);
            
                PC++;                //target next opcode
                dex();               //X <- X - 1, 1 byte long
            
                TEST(DEX_IMPL);
                break;        
            }
                
            case DEY_IMPL: 
            {
                PREPTEST(DEY_IMPL);
            
                PC++;                //target next opcode
                dey();               //Y <- Y - 1, 1 byte long
            
                TEST(DEY_IMPL);
                break;        
            }
                
                
            //############################# SHIFT & ROTATE INSTRUCTIONS #############################
                
            //############################# LOGIC INSTRUCTIONS #############################
            
            //############################# COMPARE AND TEST BIT INSTRUCTIONS #############################
                
            //############################# SET AND CLEAR INSTRUCTIONS #############################
            case SEC_IMPL: 
            {
                PREPTEST(SEC_IMPL);
            
                PC++;   //target next opcode
                sec();  //C <- 1
            
                TEST(SEC_IMPL);
                break; 
            }
            
            case SED_IMPL: 
            {
                PREPTEST(SED_IMPL);
            
                PC++;   //target next opcode
                sed();  //D <- 1
            
                TEST(SED_IMPL);
                break; 
            }
                
            case SEI_IMPL: 
            {
                PREPTEST(SEI_IMPL);
            
                PC++;   //target next opcode
                sei();  //I <- 1
            
                TEST(SEI_IMPL);
                break; 
            }
                
                
                
            case CLC_IMPL: 
            {
                PREPTEST(CLC_IMPL);
            
                PC++;   //target next opcode
                clc();  //C <- 0
                
                TEST(CLC_IMPL);
                break; 
            }
                
            case CLD_IMPL: 
            {
                PREPTEST(CLD_IMPL);
            
                PC++;   //target next opcode
                cld();  //D <- 0
            
                TEST(CLD_IMPL);
                break; 
            }
                
            case CLI_IMPL: 
            {
                PREPTEST(CLI_IMPL);
            
                PC++;   //target next opcode
                cli();  //I <- 0
            
                TEST(CLI_IMPL);
                break; 
            }
                
            case CLV_IMPL: 
            {
                PREPTEST(CLV_IMPL);
            
                PC++;   //target next opcode
                clv();  //V <- 0
            
                TEST(CLV_IMPL);
                break; 
            }
                
            //############################# JUMP AND SUBROUTINE INSTRUCTIONS #############################
                
            //############################# BRANCH INSTRUCTIONS #############################
            case BNE_REL: bne_rel(); break; //branch to PC+operand if Z == 0
                
            //############################# STACK INSTRUCTIONS #############################
                
            //############################# MISC INSTRUCTIONS #############################
            case NOP_IMPL: //do nothing, 1 byte long
            {
                PC++; //target next operand
            }
            
			default: 
            { 
                printf("Error: unknown instruction: 0x%X. Emulation stopped. \n", IR); 
                running = 0;
            }
            
            
		} //switch

	}
    
    
#ifdef TEST_MODE 
printf("\nTestmode was on.");  
#endif


	return 0;
}

//X <- A
//affects N and Z
void tax(void) //OK
{    
    X = A;                       
                
    //set flags
    setN(X);     
    setZ(X); 
}

//A <- X
//affects N and Z
void txa(void)
{
    A = X;       
    
    //set flags
    setN(A); 
    setZ(A); 
}


//Y <- A
//affects N and Z
void tay(void)
{
    Y = A;       
    
    //set flags
    setN(Y); 
    setZ(Y);
}

//A <- Y
//affects N and Z
void tya(void)
{
    A = Y;      

    //set flags
    setN(A); 
    setZ(A);    
}

//X <- SP
//affects N and Z
void tsx(void)
{    
	X = SP;       
                
    //set flags
    setN(X); 
    setZ(X);    
}

//SP <- X
//no flags
void txs(void)
{
    SP = X;      
}


//A <- M
//affects N and Z
void lda(word operand)
{      
    A = operand; 
                
    //set flags
    setN(A);
    setZ(A); 
    
    //printRegs();
}

//A -> M
//no flags
void sta(address a)
{      
    mwr(A, a); //write contents of A to address a
    //printRegs();
}

//X -> M
void stx(address a)
{      
    mwr(X, a); //write contents of X to address a
    //printRegs();
}

//Y -> M
void sty(address a)
{      
    mwr(Y, a); //write contents of Y to address a
    //printRegs();
}


//############################# ARITHMETIC INSTRUCTIONS #############################
//A <- A + M + C
//affects N, V, Z and C 
//note: decimal mode is not treated here, since NES' 6502 lacks BCD mode
void adc(word operand)
{
    word Ainit = A;                         //get initial value of A since we need it to do some checks with it later
    
    dword A16 = Ainit + operand + getC();   //store result in 16 bit int to check whether it is greater than 8 bit
    A = A16 & 0x0000FFFF;                   //copy result without cary (if exists) to A
    
    setN(A);
    
    //if +a + +b got -c or -a + -b got +c then we have an overflow here (result didn't fit into 8 bit and wrapped over)
    if ( (isN(operand) && isN(Ainit) && !isN(A)) || (!isN(operand) && !isN(Ainit) && isN(A)) )
    {
        setV(1);
    }
    
    setZ(A); 
    
    //result > 255 => 8 bits were not sufficient => need 9th bit = carry, otherwise clear carry, which might be set (and used) before
    (A16 > 0xFF) ? setC(1) : setC(0); 
    
    
   
    //TODO: test with http://skilldrick.github.io/easy6502/    
    printRegs();
}

//SBC: Subtract Memory from Accumulator with Borrow: A - M - !C -> A
//affects N, V, Z and C 
void sbc(word operand)
{
    word Ainit = A;                         //get initial value of A since we need it to do some checks with it later
    
    dword A16 = Ainit - operand - !getC();  //store result in 16 bit int to check whether it is greater than 8 bit
    A = A16 & 0x0000FFFF;                   //copy result without cary (if exists) to A
    
    setN(A);
    
    //TODO: do stuff here too
    
    //TODO: test with http://skilldrick.github.io/easy6502/    
    printRegs();
    
}

//increment X
//affects N and Z
void inx(void)
{
    X++;
    
    setN(X);
    setZ(X);        
}

//increment Y
//affects N and Z
void iny(void)
{
    Y++;
    
    setN(Y);
    setZ(Y);        
}


//decrement X
//affects N and Z
void dex(void)
{
    X--;
    
    setN(X);
    setZ(X);        
}

//decrement Y
//affects N and Z
void dey(void)
{
    Y--;
    
    setN(Y);
    setZ(Y);        
}

//C <-- 1
//affects C
void sec(void)
{
    setC(1); //set C
}

//D <-- 1
//affects D
void sed(void)
{
    setD(1); //set D
}

//I <-- 1
//affects I
void sei(void)
{
    setI(1); //set I
}


//C <-- 0
//affects C
void clc(void)
{
    setC(0); //clear C
}

//D <-- 0
//affects D
void cld(void)
{
    setD(0); //clear D
}

//I <-- 0
//affects I
void cli(void)
{
    setI(0); //clear I
}

//V <-- 0
//affects V
void clv(void)
{
    setV(0); //clear V
}

//X <- M
//affects N and Z
void ldx(word operand)
{
    X = operand;
    
    //set flags
    setN(X);
    setZ(X);    
}

//Y <- M
//affects N and Z
void ldy(word operand)
{
    Y = operand;
    
    //set flags
    setN(Y);
    setZ(Y);    
}

//TODO: what's with this one? CHECK!
//branch to PC+operand if result is not zero, i.e. Z == 0
//2 bytes long, no flags affected
void bne_rel(void)
{
    sword operand = mrd(PC+1);  //target operand 1 (it's the offset, which is in [-128, 127])    
    PC = PC+2;                  //target next opcode, target will be overwritten if branch is taken
    
    //printf("before branch\n");
    if (getZ() == 0) 
    {
        //mdump(0, 10);
        //printf("Z == 0, going to branch\n");
        printRegs();
        //printf("offset is: %i", operand);
        PC = PC+(operand); //PC <- PC+operand, operand is in [-128, 127]        
    }
}


