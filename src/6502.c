/***********************************
*** 6502 CPU emulator main file. ***
************************************/

#include <stdio.h>
#include <stdlib.h>
#include "6502.h"
#include "mem.h"
#include "utils.h"
#include "test.h"

#define START_ADDRESS 0x0000    //start address of the programm (PC init)
#define STACK_MIN 0x01FF        //stack grows downwards starting at this address
#define STACK_MAX 0x0100        //end of stack range, next lower address results in stack overflow

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
address SP; //6502's stack has a range of 256 and is hard wired to 2nd memory page 0100 to 01FF (9 bit address)
address PC; //program counter, NOTE: PC contains always the instruction to be fetched next !!!


//set initial register values
void reset()
{
    X   =   0x0;
    Y   =   0x0;
    A   =   0x0;
    P   =   0x30; //00110000 = (N V - B D I Z C) // - always 1, B is 1 too because NES does not use decimal mode D at all
    IR  =   0x0;    
    SP  =   STACK_MIN;  
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
void setV(uint8_t flag)
{
    if (flag >= 1) P = P | 0b01000000; //V = 1 => overflow occured
    else P = P & 0b10111111; 
}

//set B in P = (N V - B D I Z C) 
void setB(uint8_t flag)
{
    if (flag >= 1) P = P | 0b00010000; 
    else P = P & 0b11101111; 
}

//set D in P = (N V - B D I Z C) 
void setD(uint8_t flag)
{
    if (flag >= 1) P = P | 0b00001000; 
    else P = P & 0b11110111; 
}

//set I in P = (N V - B D I Z C) 
void setI(uint8_t flag)
{
    if (flag >= 1) P = P | 0b00000100; 
    else P = P & 0b11111011; 
}

//set Z in P = (N V - B D I Z C)
void setZ(word reg)
{
    if (reg == 0) P = P | 0b00000010; //register is zero => set Z to 1
    else P = P & 0b11111101;          //register is non zero => set Z to 0
}

//set C in P = (N V - B D I Z C) 
void setC(uint8_t flag)
{
    if (flag >= 1) P = P | 0b00000001; 
    else P = P & 0b11111110; 
}

//get N from P = (N V - B D I Z C)
word getN(void)
{
    //move N bit to the right most location and clear all bits bevore N => result is 0 or 1
    word n = (P >> 7) & 0b00000001; 
    return n;
}

//get V from P = (N V - B D I Z C)
word getV(void)
{
    //move V bit to the right most location and clear all bits bevore V => result is 0 or 1
    word v = (P >> 6) & 0b00000001; 
    return v;
}

//get B from P = (N V - B D I Z C)
word getB(void)
{
    //move B bit to the right most location and clear all bits bevore B => result is 0 or 1
    word b = (P >> 4) & 0b00000001; 
    return b;
}

//get D from P = (N V - B D I Z C)
word getD(void)
{
    //move D bit to the right most location and clear all bits bevore D => result is 0 or 1
    word d = (P >> 3) & 0b00000001; 
    return d;
}

//get I from P = (N V - B D I Z C)
word getI(void)
{
    //move I bit to the right most location and clear all bits bevore I => result is 0 or 1
    word i = (P >> 2) & 0b00000001; 
    return i;
}

//get Z from P = (N V - B D I Z C) 
word getZ(void)
{
    //move Z bit to the right most location and clear all bits bevore Z => result is 0 or 1
    word z = (P >> 1) & 0b00000001; 
    return z;
}

//get C from P = (N V - B D I Z C)
word getC(void)
{
    //clear all bits bevore C => result is 0 or 1
    word c = P & 0b00000001; 
    return c;
}

//checks whether specified word is negative or not
word isN(word w)
{
    //move bit 7 to the right most location and clear all bits 0 - 6 => result is 0 or 1
    word n = (w >> 7) & 0b00000001; 
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
    word lo = mrd(PC+1);                //get least significant byte of operand address (little endian)
    word hi = mrd(PC+2);                //get most significant byte of operand address (little endian)
    return lohi2addr(lo,hi);            //convert lo byte and hi byte to a 16bit address    
}

//NOTE: no wrapping here if final address > 16bit, 6502 programmer must care by himself!!!
address getAbsXAddr(void)
{
    word lo = mrd(PC+1);                //get least significant byte of operand address (little endian)
    word hi = mrd(PC+2);                //get most significant byte of operand address (little endian)
    return lohi2addr(lo,hi) + X;        //convert lo byte and hi byte to a 16bit address and add X 
}

//NOTE: no wrapping here if final address > 16bit, 6502 programmer must care by himself!!!
address getAbsYAddr(void)
{
    word lo = mrd(PC+1);                //get least significant byte of operand address (little endian)
    word hi = mrd(PC+2);                //get most significant byte of operand address (little endian)
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
//The little-endian address stored at the two-byte pair of sum address (LSB) and sum address plus one (MSB) 
//is loaded and the value at that address is used to perform the computation.
//Example:
//The value $02 in X is added to $15 for a sum of $17. 
//The address $D010 at addresses $0017 and $0018 will be where the value $0F in the Accumulator is stored.
//zeropage+X wraps to an address in zeropage again
//in short: word operand = *(mem[PC+1]+X)
word getXIndOp(void)
{
    word zrp_addr = mrd(PC+1);                              //get base address from zeropage   
    word zrp_addrx = (zrp_addr + X) & 0xFF;                 //add X offset and wrap to zeropage    
    word operand_ptr_lo = zrp_addrx;                        //get lo part of the operand address address
    word operand_ptr_hi = zrp_addrx + 1;                    //get hi part of the operand address address
    word operand_addr_lo = mrd(operand_ptr_lo);             //get lo part of the operand address
    word operand_addr_hi = mrd(operand_ptr_hi);             //get hi part of the operand address 
    address a = lohi2addr(operand_addr_lo, operand_addr_hi);//convert lo byte and hi byte to a 16bit address
    word operand = mrd(a);                                  //finally, get operand value
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
    a = a + Y;                                          //add Y to calculated address
    word operand = mrd(a);                                      //finally, get operand
    return operand;
}

//just in case, print a brief warning that opcode <opcode_name> at address <opcode_address> caused a stack overflow
void warnStackOverflow(const char* opcode_name, address opcode_address)
{
    if (SP < STACK_MAX) 
        printf("WARNING: %s instruction at 0x%.4X resulted in stack overflow.\nStack pointer is now at: 0x%.4X.\n\n", opcode_name, opcode_address, SP);
}

//just in case, print a brief warning that opcode <opcode_name> at address <opcode_address> caused a stack underflow
void warnStackUnderflow(const char* opcode_name, address opcode_address)
{
    if (SP > STACK_MIN)    
        printf("WARNING: %s instruction at 0x%.4X resulted in stack underflow.\nStack pointer is now at: 0x%.4X.\n\n", opcode_name, opcode_address, SP);
}


    

//load given 6502 binary and emulate it
int main(int argc, char *argv[])
{	
    
    //exit if path to binary was not given
    if (argc < 2) 
    {
        printf("Input error: one argument expected: path to 6502-binary \n");
        return -1;
    }
    
    //load binary into emu-RAM
    int load_status = load(argv[1]);
    
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
            case LDA_XIND: //A <- M [TODO], 2 bytes long
            {
                PREPTEST(LDA_XIND);

                word operand = getXIndOp();
                PC += 2;
                lda(operand);

                TEST(LDA_XIND);
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
            
                word operand = getZrpOp();      //get operand from zeropage 
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
            
                word operand = getZrpOp();      //get operand from zeropage 
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
            case LDA_XINDX_TEMPLATE: //A -> M [TODO], 2 bytes long
            {
                word operand = getXIndOp();
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
                
            //TODO: other SBCs here
            
                
            case INC_ZRP: //2 bytes long
            {
                PREPTEST(INC_ZRP);

                address a = getZrpAddr();       //get address from zeropage                 
                PC += 2;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ZRP);
                break;
            }
            
            case INC_ZRPX: //2 bytes long
            {
                PREPTEST(INC_ZRPX);

                address a = getZrpXAddr();      //get address from zeropage + X                 
                PC += 2;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ZRPX);
                break;
            }

            case INC_ABS: //3 bytes long
            {
                PREPTEST(INC_ABS);

                address a = getAbsAddr();       //get absolute address                
                PC += 3;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ABS);
                break;
            }

            case INC_ABSX: //3 bytes long
            {
                PREPTEST(INC_ABSX);

                address a = getAbsXAddr();      //get absolute address                
                PC += 3;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ABSX);
                break;
            }
            
            
            case INX_IMPL: 
            {
                PREPTEST(INX_IMPL);
            
                PC++;                //target next opcode
                inx();               //execute opcode
            
                TEST(INX_IMPL);
                break;        
            }
                
            case INY_IMPL: 
            {
                PREPTEST(INY_IMPL);
            
                PC++;                //target next opcode
                iny();               //execute opcode
            
                TEST(INY_IMPL);
                break;        
            }
                
            case DEX_IMPL: 
            {
                PREPTEST(DEX_IMPL);
            
                PC++;                //target next opcode
                dex();               //execute opcode
            
                TEST(DEX_IMPL);
                break;        
            }
                
            case DEY_IMPL: 
            {
                PREPTEST(DEY_IMPL);
            
                PC++;                //target next opcode
                dey();               //execute opcode
            
                TEST(DEY_IMPL);
                break;        
            }
                
                
            //############################# SHIFT & ROTATE INSTRUCTIONS #############################
            case ASL_ACCU: 
            {
                PREPTEST(ASL_ACCU);
            
                PC++;                //target next opcode
                asl_accu();          //execute opcode
            
                TEST(ASL_ACCU);
                break;        
            }

            case ASL_ZRP: 
            {
                PREPTEST(ASL_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                PC += 2;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ZRP);
                break;        
            }
            
            case ASL_ZRPX: 
            {
                PREPTEST(ASL_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                PC += 2;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ZRPX);
                break;        
            }

            case ASL_ABS: 
            {
                PREPTEST(ASL_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                PC += 3;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ABS);
                break;        
            }

            case ASL_ABSX: 
            {
                PREPTEST(ASL_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                PC += 3;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ABSX);
                break;        
            }

            case LSR_ACCU: 
            {
                PREPTEST(LSR_ACCU);
            
                PC++;                           //target next opcode
                lsr_accu();                     //execute opcode
            
                TEST(LSR_ACCU);
                break;        
            }

            case LSR_ZRP: 
            {
                PREPTEST(LSR_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                PC += 2;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ZRP);
                break;        
            }
            
            case LSR_ZRPX: 
            {
                PREPTEST(LSR_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                PC += 2;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ZRPX);
                break;        
            }

            case LSR_ABS: 
            {
                PREPTEST(LSR_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                PC += 3;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ABS);
                break;        
            }

            case LSR_ABSX: 
            {
                PREPTEST(LSR_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                PC += 3;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ABSX);
                break;        
            }

            case ROL_ACCU: 
            {
                PREPTEST(ROL_ACCU);
            
                PC++;                //target next opcode
                rol_accu();          //execute opcode
            
                TEST(ROL_ACCU);
                break;        
            }

            case ROL_ZRP: 
            {
                PREPTEST(ROL_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                PC += 2;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ZRP);
                break;        
            }
            
            case ROL_ZRPX: 
            {
                PREPTEST(ROL_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                PC += 2;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ZRPX);
                break;        
            }

            case ROL_ABS: 
            {
                PREPTEST(ROL_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                PC += 3;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ABS);
                break;        
            }

            case ROL_ABSX: 
            {
                PREPTEST(ROL_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                PC += 3;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ABSX);
                break;        
            }
            
                
            case ROR_ACCU: 
            {
                PREPTEST(ROR_ACCU);
            
                PC++;                //target next opcode
                ror_accu();          //execute opcode
            
                TEST(ROR_ACCU);
                break;        
            }

            case ROR_ZRP: 
            {
                PREPTEST(ROR_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                PC += 2;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ZRP);
                break;        
            }
            
            case ROR_ZRPX: 
            {
                PREPTEST(ROR_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                PC += 2;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ZRPX);
                break;        
            }

            case ROR_ABS: 
            {
                PREPTEST(ROR_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                PC += 3;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ABS);
                break;        
            }

            case ROR_ABSX: 
            {
                PREPTEST(ROR_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                PC += 3;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ABSX);
                break;        
            }

            //############################# LOGIC INSTRUCTIONS #############################
            case AND_IMMD: 
            {
                PREPTEST(AND_IMMD);
                
                word operand = getImdOp();      //get immediate operand
                PC += 2;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_IMMD);
                break;        
            }

            case AND_ZRP: 
            {
                PREPTEST(AND_ZRP);
                
                word operand = getZrpOp();      //get zeropage operand
                PC += 2;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ZRP);
                break;        
            }

            case AND_ZRPX: 
            {
                PREPTEST(AND_ZRPX);
                
                word operand = getZrpXOp();     //get zeropage+X operand
                PC += 2;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ZRPX);
                break;        
            }

            case AND_ABS: 
            {
                PREPTEST(AND_ABS);
                
                word operand =  getAbsOp();     //get absoulte operand
                PC += 3;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ABS);
                break;        
            }

            case AND_ABSX: 
            {
                PREPTEST(AND_ABSX);
                
                word operand =  getAbsXOp();    //get absolute+X operand
                PC += 3;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ABSX);
                break;        
            }

            case AND_ABSY: 
            {
                PREPTEST(AND_ABSY);
                
                word operand =  getAbsYOp();    //get absolute+Y operand
                PC += 3;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ABSY);
                break;        
            }


            //############################# COMPARE AND TEST BIT INSTRUCTIONS #############################
                
            //############################# SET AND CLEAR INSTRUCTIONS #############################
            case SEC_IMPL: 
            {
                PREPTEST(SEC_IMPL);
            
                PC++;   //target next opcode
                sec();  //execute opcode
            
                TEST(SEC_IMPL);
                break; 
            }
            
            case SED_IMPL: 
            {
                PREPTEST(SED_IMPL);
            
                PC++;   //target next opcode
                sed();  //execute opcode
            
                TEST(SED_IMPL);
                break; 
            }
                
            case SEI_IMPL: 
            {
                PREPTEST(SEI_IMPL);
            
                PC++;   //target next opcode
                sei();  //execute opcode
            
                TEST(SEI_IMPL);
                break; 
            }
                
                
                
            case CLC_IMPL: 
            {
                PREPTEST(CLC_IMPL);
            
                PC++;   //target next opcode
                clc();  //execute opcode
                
                TEST(CLC_IMPL);
                break; 
            }
                
            case CLD_IMPL: 
            {
                PREPTEST(CLD_IMPL);
            
                PC++;   //target next opcode
                cld();  //execute opcode
            
                TEST(CLD_IMPL);
                break; 
            }
                
            case CLI_IMPL: 
            {
                PREPTEST(CLI_IMPL);
            
                PC++;   //target next opcode
                cli();  //execute opcode
            
                TEST(CLI_IMPL);
                break; 
            }
                
            case CLV_IMPL: 
            {
                PREPTEST(CLV_IMPL);
            
                PC++;   //target next opcode
                clv();  //execute opcode
            
                TEST(CLV_IMPL);
                break; 
            }
                
            //############################# JUMP AND SUBROUTINE INSTRUCTIONS #############################

            case JMP_ABS: //TODO
            {
                PREPTEST(JMP_ABS);
            
                // PC++;   //target next opcode
                // jmp();  //execute opcode
            
                TEST(JMP_ABS);
                break; 
            }

            case JMP_IND: //TODO
            {
                PREPTEST(JMP_IND);
            
                // PC++;   //target next opcode
                // jmp();  //execute opcode
            
                TEST(JMP_IND);
                break; 
            }

            case JSR_ABS:
            {
                PREPTEST(JSR_ABS);

                address a = getAbsAddr();       //get absoulte address
                PC += 2;                        //it's 3-byte opcode but we must increment only by 2 (corresponding RTS will increment the PC later)
                jsr(a);                         //execute opcode
            
                TEST(JSR_ABS);
                break; 
            }

            case RTS_IMPL:
            {
                PREPTEST(RTS_IMPL);
                            
                rts();  //execute opcode
                PC++;   //PC must be incremented after pulling it from stack 
                        //because it still points to JSR's 2nd parameter rather than to the next opcode
            
                TEST(RTS_IMPL);
                break; 
            }

            case RTI_IMPL: //TODO
            {
                PREPTEST(RTI_IMPL);
            
                // PC++;   //target next opcode
                // jsr();  //execute opcode
            
                TEST(RTI_IMPL);
                break; 
            }                      
                            
            //############################# BRANCH INSTRUCTIONS #############################
            case BNE_REL: 
            {
                bne_rel(); //branch to PC+operand if Z == 0
                break;
            }
                
            //############################# STACK INSTRUCTIONS #############################
            case PHA_IMPL:
            {
                PREPTEST(PHA_IMPL);

                PC++;
                pha_impl();

                TEST(PHA_IMPL);
                break;           
            }    

            case PLA_IMPL:
            {
                PREPTEST(PLA_IMPL);

                PC++;
                pla_impl();

                TEST(PLA_IMPL);
                break;           
            } 

            case PHP_IMPL:
            {
                PREPTEST(PHP_IMPL);

                PC++;
                php_impl();

                TEST(PHP_IMPL);
                break;           
            } 

            case PLP_IMPL:
            {
                PREPTEST(PLP_IMPL);

                PC++;
                plp_impl();

                TEST(PLP_IMPL);
                break;           
            } 


            //############################# MISC INSTRUCTIONS #############################
            case NOP_IMPL: //do nothing, 1 byte long
            {
                PC++; //nothing to do, just target next operand
                break;
            }


            //############################# MISC INSTRUCTIONS #############################
            case 0x0: //no instruction found, assuming that program has exited
            {
                printf("\nNo more instructions. Emulation stopped. \n");
                running = 0;
                break;
            }
            
			default: //invalid instruction
            { 
                printf("\nError: unknown instruction: 0x%X at 0x%.4X. Emulation stopped. \n", IR, PC); 
                running = 0;
            }            
            
		} //switch IR

	} //while running
    
    
#ifdef TEST_MODE 
printf("\nTestmode was on.\n");  
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
}

//A -> M
//no flags
void sta(address a)
{      
    mwr(A, a); //write contents of A to address a   
}

//X -> M
void stx(address a)
{      
    mwr(X, a); //write contents of X to address a    
}

//Y -> M
void sty(address a)
{      
    mwr(Y, a); //write contents of Y to address a    
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

//increment memory: M <- M + 1
//affects N and Z
void inc(address a)
{
    word w = mrd(a);
    mwr(++w, a);

    setN(w);
    setZ(w);
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

//A <- (A << 1), original bit #7 is stored to carry flag
//affects N, Z, C
void asl_accu(void)
{   
    setC(getBit(A, 7)); //before shifting, save bit #7 to carry
    
    A = A << 1; //the actual shift operation   

    setN(A);
    setZ(A); 
}

//M[a] <- (M[a] << 1), original bit #7 is stored to carry flag
//affects N, Z, C
void asl(address a)
{   
    word w = mrd(a); //get word stored at address

    setC(getBit(w, 7)); //before shifting, save bit #7 to carry
    
    w = w << 1; //the actual shift operation

    mwr(w, a); //write back updated word

    setN(w);
    setZ(w); 
}

//A <- (A >> 1), original bit #0 is stored to carry flag
//affects N, Z, C
void lsr_accu(void)
{   
    setC(getBit(A, 0)); //before shifting, save bit #0 to carry
    
    A = A >> 1; //the actual shift operation   

    setN(A);
    setZ(A); 
}

//M[a] <- (M[a] >> 1), original bit #0 is stored to carry flag
//affects N, Z, C
void lsr(address a)
{   
    word w = mrd(a); //get word stored at address

    setC(getBit(w, 0)); //before shifting, save bit #0 to carry
    
    w = w >> 1; //the actual shift operation

    mwr(w, a); //write back updated word

    setN(w);
    setZ(w); 
}

//rotate left: shift A left, copy original bit #7 to carry and to bit #0 of A
//affects N, Z, C
void rol_accu(void)
{   
    setC(getBit(A, 7)); //before shifting, save bit #7 to carry
    
    A = A << 1; //the actual shift operation

    A = A | getC(); //copy carry to bit #0

    setN(A);
    setZ(A); 
}

//rotate left: shift M[a] left, copy original bit #7 to carry and to bit #0 of M[a]
//affects N, Z, C
void rol(address a)
{   
    word w = mrd(a); //get word stored at address

    setC(getBit(w, 7)); //before shifting, save bit #7 to carry
    
    w = w << 1; //the actual shift operation

    w = w | getC(); //copy carry to bit #0

    mwr(w, a); //write back updated word

    setN(w);
    setZ(w); 
}

//rotate right: shift A right, copy original bit #0 to carry and to bit #7 of A
//affects N, Z, C
void ror_accu(void)
{   
    setC(getBit(A, 0)); //before shifting, save bit #0 to carry
    
    A = A >> 1; //the actual shift operation

    A = A | (getC() << 7); //copy carry to bit #7

    setN(A);
    setZ(A); 
}

//rotate right: shift M[a] rigth, copy original bit #0 to carry and to bit #7 of M[a]
//affects N, Z, C
void ror(address a)
{   
    word w = mrd(a); //get word stored at address

    setC(getBit(w, 0)); //before shifting, save bit #0 to carry
    
    w = w >> 1; //the actual shift operation

    w = w | (getC() << 7); //copy carry to bit #7

    mwr(w, a); //write back updated word

    setN(w);
    setZ(w); 
}

//A <-- A & operand
//affects N, Z
void and(word operand)
{
    A = A & operand;

    setN(A);
    setZ(A);
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

//jump to subroutine: push PC to stack and load PC with jump address a
//note: JSR instruction increments the PC only by 2 (according to real HW implementation)
//the PC is incremented to proper address later by corresponding RTS
//no flags affected
void jsr(address a)
{
    mwr((PC & 0xFF00)>>2, SP);  //push PC-HI to stack

    SP--;                       //point to next free stack location

    mwr(PC & 0x00FF, SP);       //push PC-LO to stack

    SP--;                       //point to next free stack location
    
    PC = a;                     //store jump address to PC

    //pushing beyond MAX?
    warnStackOverflow("JSR", PC-2);
}

//return from subroutine: pull previously saved PC value from stack and loat it into PC register
//note: after pulling the PC from stack it must be incremented (according to real HW implementation)
//no flags affected
void rts(void)
{    
    SP++;                       //target value on stack that will be pulled

    word pclo = mrd(SP);        //pull value from stack, the value should be LO byte of the previously pushed PC register
    
    SP++;                       //target next stack value that will be pulled
    
    word pchi = mrd(SP);        //pull value from stack, the value should be HI byte of the previously pushed PC register

    PC = lohi2addr(pclo, pchi); //from LO byte and HI byte, construct address and store it into PC register (PC is then restored after JSR)

    //pulling beyond MIN?
    warnStackUnderflow("RTS", PC);
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

//push A to stack, i.e. mem[SP] <- A
//no flags affected
void pha_impl(void)
{
    mwr(A, SP);     //push A to stack
    SP--;           //point to next free stack location
    
    warnStackOverflow("PHA", PC-1); //pushing beyond MAX?
}

//pull value from stack into A, i.e. A <- mem[SP+1]
//affects N and Z
void pla_impl(void)
{
    SP++;           //target value on top of the stack

    A = mrd(SP);    //copy value from stack into A, value in SP is free to be overwritten by next push operation

    //set flags
    setN(A);
    setZ(A); 

    warnStackUnderflow("PLP", PC-1); //pushing beyond MIN?
}

//push P to stack, i.e. mem[SP] <- P
//no flags affected
void php_impl(void)
{
    mwr(P, SP); //push P to stack
    SP--;       //point to next free stack location

    warnStackOverflow("PHP", PC-1); //pushing beyond MAX?
}

//pull value from stack into P, i.e. P <- mem[SP+1]
//affects all bits in P, because a new value is fetched into P
void plp_impl(void)
{
    SP++;        //target value on top of the stack

    P = mrd(SP); //copy value from stack into P, value in SP is free to be overwritten by next push operation

    warnStackUnderflow("PLP", PC-1); //pushing beyond MIN?
}
                 


