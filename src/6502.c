/***********************************
*** 6502 CPU emulator core file. ***
************************************/

#include <stdio.h>
#include <stdlib.h>
#include "6502.h"
#include "mem.h"
#include "utils.h"
//#include "test.h"


#define PREPTEST(opcode)    //expand to nothing
#define TEST(opcode)    //expand to nothing

#define ENABLE_DBG_TRACE //dbg: print executed opcodes

#ifdef ENABLE_DBG_TRACE
    #define DBG_TRACE(opcode) printExecInfo(TAX_IMPL);
#else
    #define DBG_TRACE(opcode) //expand to nothing
#endif


#define START_ADDRESS 0x0000    //start address of the programm (PC init)
#define STACK_MIN 0x01FF        //stack grows downwards starting at this address
#define STACK_MAX 0x0100        //end of stack range, next lower address results in stack overflow


//allocate cpu struct and store cpu and memory to global variables 
T6502 cpuInit(TMemory mem) {

    T6502 cpu = (T6502)malloc(sizeof(CpuStruct));
    cpu->X = 0;
    cpu->Y = 0;
    cpu->A = 0;
    cpu->P = 0x30; //00110000 = (N V - B D I Z C) // - always 1, B is 1 too because NES does not use decimal mode D at all
    cpu->IR = 0;
    cpu->SP = STACK_MIN;
    cpu->PC = START_ADDRESS;
    cpu->mem = mem;

    return cpu;
}


//set N in P = (N V - B D I Z C) if word value is negative
void setNByWord(T6502 cpu, word w)
{
    word tmp = w >> 7; //shift word value 7 bits to right => get only 1st bit, i.e. 0b00000001
    
    if (tmp == 1) 
    {
        cpu->P = cpu->P | 0b10000000; //N = 1 => word value is negative
    }
    else 
    {
        cpu->P = cpu->P & 0b01111111; //N = 0 => word value is non negative        
    }
}

//set N in P = (N V - B D I Z C) if flag is != 0
void setNByFlag(T6502 cpu, uint8_t flag)
{
    if (flag >= 1) 
    {
        cpu->P = cpu->P | 0b10000000; //N = 1
    }
    else 
    {
        cpu->P = cpu->P & 0b01111111; //N = 0
    }
}

//set V in P = (N V - B D I Z C)
void setVByFlag(T6502 cpu, uint8_t flag)
{
    if (flag >= 1) cpu->P = cpu->P | 0b01000000; //V = 1 => overflow occured
    else cpu->P = cpu->P & 0b10111111; 
}

//set B in P = (N V - B D I Z C) 
void setBByFlag(T6502 cpu, uint8_t flag)
{
    if (flag >= 1) cpu->P = cpu->P | 0b00010000; 
    else cpu->P = cpu->P & 0b11101111; 
}

//set D in P = (N V - B D I Z C) 
void setDByFlag(T6502 cpu, uint8_t flag)
{
    if (flag >= 1) cpu->P = cpu->P | 0b00001000; 
    else cpu->P = cpu->P & 0b11110111; 
}

//set I in P = (N V - B D I Z C) 
void setIByFlag(T6502 cpu, uint8_t flag)
{
    if (flag >= 1) cpu->P = cpu->P | 0b00000100; 
    else cpu->P = cpu->P & 0b11111011; 
}

//set Z in P = (N V - B D I Z C)
void setZByWord(T6502 cpu, word w)
{
    if (w == 0) cpu->P = cpu->P | 0b00000010; //word is zero => set Z to 1
    else cpu->P = cpu->P & 0b11111101;        //word is non zero => set Z to 0
}

//set Z in P = (N V - B D I Z C)
void setZByFlag(T6502 cpu, uint8_t flag)
{
    if (flag >= 0) cpu->P = cpu->P | 0b00000010;  //set Z to 1
    else cpu->P = cpu->P & 0b11111101;            //set Z to 0
}

//set C in P = (N V - B D I Z C) 
void setCByFlag(T6502 cpu, uint8_t flag)
{
    if (flag >= 1) cpu->P = cpu->P | 0b00000001; 
    else cpu->P = cpu->P & 0b11111110; 
}

//get N from P = (N V - B D I Z C)
word getN(T6502 cpu)
{
    //move N bit to the right most location and clear all bits bevore N => result is 0 or 1
    word n = (cpu->P >> 7) & 0b00000001; 
    return n;
}

//get V from P = (N V - B D I Z C)
word getV(T6502 cpu)
{
    //move V bit to the right most location and clear all bits bevore V => result is 0 or 1
    word v = (cpu->P >> 6) & 0b00000001; 
    return v;
}

//get B from P = (N V - B D I Z C)
word getB(T6502 cpu)
{
    //move B bit to the right most location and clear all bits bevore B => result is 0 or 1
    word b = (cpu->P >> 4) & 0b00000001; 
    return b;
}

//get D from P = (N V - B D I Z C)
word getD(T6502 cpu)
{
    //move D bit to the right most location and clear all bits bevore D => result is 0 or 1
    word d = (cpu->P >> 3) & 0b00000001; 
    return d;
}

//get I from P = (N V - B D I Z C)
word getI(T6502 cpu)
{
    //move I bit to the right most location and clear all bits bevore I => result is 0 or 1
    word i = (cpu->P >> 2) & 0b00000001; 
    return i;
}

//get Z from P = (N V - B D I Z C) 
word getZ(T6502 cpu)
{
    //move Z bit to the right most location and clear all bits bevore Z => result is 0 or 1
    word z = (cpu->P >> 1) & 0b00000001; 
    return z;
}

//get C from P = (N V - B D I Z C)
word getC(T6502 cpu)
{
    //clear all bits bevore C => result is 0 or 1
    word c = cpu->P & 0b00000001; 
    return c;
}

//checks whether specified word is negative or not
word isN(word w)
{
    //move bit 7 to the right most location and clear all bits 0 - 6 => result is 0 or 1
    word n = (w >> 7) & 0b00000001; 
    return n;
}

address getZrpAddr(T6502 cpu)
{
    address a = memRead(cpu->mem, cpu->PC+1);              //get address from zeropage
    return a;
}

//The address calculation wraps around if the sum of the base address and the register exceed $FF (e.g. $80 + $FF => $7F) and not $017F.
address getZrpXAddr(T6502 cpu)
{
    address a = memRead(cpu->mem, cpu->PC+1);              //get address from zeropage    
    a = (a + cpu->X) & 0x00FF;               //address must be within zero page => wrap around if a+X > 8bit address (i.e. clear MSB);            
    return a; 
}

address getZrpYAddr(T6502 cpu)
{
    address a = memRead(cpu->mem, cpu->PC+1);              //get address from zeropage    
    a = (a + cpu->Y) & 0x00FF;               //address must be within zero page => wrap around if a+Y > 8bit address;            
    return a; 
}

//operand is absolute address which is stored in little endian format, e.g. CDAB
//the actual mem address we want to access is ABCD though, hence convert little endian CDAB to address ABCD, then we can do mem[ABCD]
address getAbsAddr(T6502 cpu)
{
    word lo = memRead(cpu->mem, cpu->PC+1);                //get least significant byte of operand address (little endian)
    word hi = memRead(cpu->mem, cpu->PC+2);                //get most significant byte of operand address (little endian)
    return lohi2addr(lo,hi);            //convert lo byte and hi byte to a 16bit address    
}

//NOTE: no wrapping here if final address > 16bit, 6502 programmer must care by himself!!!
address getAbsXAddr(T6502 cpu)
{
    word lo = memRead(cpu->mem, cpu->PC+1);                //get least significant byte of operand address (little endian)
    word hi = memRead(cpu->mem, cpu->PC+2);                //get most significant byte of operand address (little endian)
    return lohi2addr(lo,hi) + cpu->X;        //convert lo byte and hi byte to a 16bit address and add X 
}

//NOTE: no wrapping here if final address > 16bit, 6502 programmer must care by himself!!!
address getAbsYAddr(T6502 cpu)
{
    word lo = memRead(cpu->mem, cpu->PC+1);                //get least significant byte of operand address (little endian)
    word hi = memRead(cpu->mem, cpu->PC+2);                //get most significant byte of operand address (little endian)
    return lohi2addr(lo,hi) + cpu->Y;        //convert lo byte and hi byte to a 16bit address and add Y 
}

word getImdOp(T6502 cpu)
{
    word operand = memRead(cpu->mem, cpu->PC+1);
    return operand;
}

word getZrpOp(T6502 cpu)
{
    address a = getZrpAddr(cpu);           //get operand address within zero page, i.e. 8 bit address
    word operand = memRead(cpu->mem, a);              //get operand
    return operand;
}

word getZrpXOp(T6502 cpu)
{
    address a = getZrpXAddr(cpu);          //get operand address within zero page + X, i.e. 8 bit address
    word operand = memRead(cpu->mem, a);              //get operand
    return operand;
}

word getZrpYOp(T6502 cpu)
{
    address a = getZrpYAddr(cpu);          //get operand address within zero page + Y, i.e. 8 bit address
    word operand = memRead(cpu->mem, a);              //get operand
    return operand;
}

word getAbsOp(T6502 cpu)
{
    address a = getAbsAddr(cpu);           //get absolute address, i.e. PC+1 concat PC+2
    word operand = memRead(cpu->mem, a);              //get operand
    return operand;
}

word getAbsXOp(T6502 cpu)
{
    address a = getAbsXAddr(cpu);          //get absolute address + X, i.e. (PC+1 concat PC+2) + X
    word operand = memRead(cpu->mem, a);              //get operand
    return operand;
}

word getAbsYOp(T6502 cpu)
{
    address a = getAbsYAddr(cpu);          //get absolute address + Y, i.e. (PC+1 concat PC+2) + Y
    word operand = memRead(cpu->mem, a);              //get operand
    return operand;
}

sword getRelOp(T6502 cpu)
{
    word operand = memRead(cpu->mem, cpu->PC+1);
    return (sword) operand;
}

//Indexing first, then indirection:
//A1 is an 8bit zeropage address located at mem[PC+1].
//A2 = A1+X is a zeropage address that contains the lo-byte of the 16bit target address (location of operand to be fetched).
//Note: A2 must remain within zeropage (A2 &= 0xFF).
word getXIndOp(T6502 cpu)
{
    word zrp_addr = memRead(cpu->mem, cpu->PC+1);                              //get base address from zeropage   
    word zrp_addrx = (zrp_addr + cpu->X) & 0xFF;                 //add X offset and wrap to zeropage    
    word operand_ptr_lo = zrp_addrx;                        //get lo part of the operand address address
    word operand_ptr_hi = zrp_addrx + 1;                    //get hi part of the operand address address
    word operand_addr_lo = memRead(cpu->mem, operand_ptr_lo);             //get lo part of the operand address
    word operand_addr_hi = memRead(cpu->mem, operand_ptr_hi);             //get hi part of the operand address 
    address a = lohi2addr(operand_addr_lo, operand_addr_hi);//convert lo byte and hi byte to a 16bit address
    word operand = memRead(cpu->mem, a);                                  //finally, get operand value
    return operand;
}

//Indirection first, then indexing:
//A1 is a 16bit address, whose lo-byte is located at mem[PC+1].
//A2 = A1+Y is the 16bit address that contains the operand to be fetched (=pointer).
address getIndYOp(T6502 cpu)
{
    word zrp_addr = memRead(cpu->mem, cpu->PC+1);                              //get address, it's an 8bit zero page address   
    word operand_addr_lo = zrp_addr;                        //get lo part of the operand address
    word operand_addr_hi = zrp_addr + 1;                    //get hi part of the operand address
    address a = lohi2addr(operand_addr_lo, operand_addr_hi);//convert lo byte and hi byte to a 16bit address
    a = a + cpu->Y;                                              //add Y offset to calculated address
    word operand = memRead(cpu->mem, a);                                  //finally, get operand
    return operand;
}

//just in case, print a brief warning that opcode <opcode_name> at address <opcode_address> caused a stack overflow
void warnStackOverflow(T6502 cpu, const char* opcode_name, address opcode_address)
{
    if (cpu->SP < STACK_MAX) 
        printf("WARNING: %s instruction at 0x%.4X resulted in stack overflow.\nStack pointer is now at: 0x%.4X.\n\n", opcode_name, opcode_address, cpu->SP);
}

//just in case, print a brief warning that opcode <opcode_name> at address <opcode_address> caused a stack underflow
void warnStackUnderflow(T6502 cpu, const char* opcode_name, address opcode_address)
{
    if (cpu->SP > STACK_MIN)    
        printf("WARNING: %s instruction at 0x%.4X resulted in stack underflow.\nStack pointer is now at: 0x%.4X.\n\n", opcode_name, opcode_address, cpu->SP);
}

// ################################ begin opcode implementation ################################

//X <- A
//affects N and Z
void tax(T6502 cpu) //OK
{    
    cpu->X = cpu->A;
                
    //set flags
    setNByWord(cpu, cpu->X);
    setZByWord(cpu, cpu->X); 
}


// ################################# end opcode implementation #################################


//here we go: fetch, decode, execute
eCpuStepStatus cpuStep(T6502 cpu)
{
    if (cpu == NULL)
    {
        printf("\nError: CPU was not inited");
        return CPU_STEP_ERROR;
    }

    //fetch 
    cpu->IR = memRead(cpu->mem, cpu->PC);
    
    //decode, then execute
    switch(cpu->IR)
    {
        //############################# TRANSFER INSTRUCTIONS #############################
        case TAX_IMPL:  //X <- A, 1 byte long
        {
            DBG_TRACE(TAX_IMPL);
            cpu->PC++;                           //target next opcode
            tax(cpu);                          //execute opcode      
                    
            return CPU_STEP_OK;
        }

        default: //invalid instruction
        { 
            printf("\nError: unknown instruction: 0x%X at 0x%.4X.\n", cpu->IR, cpu->PC);
            return CPU_STEP_ERROR;
        }            
        
	} //switch IR
    
}
    
#ifdef OMITFORNOW

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
    //int load_status = load(argv[1]);
    
    //exit if loading failed
    //if (load_status != 0) return -1;
    
    //init registers
    //reset();
	
    //initially, processor is running
	int running = 1;
    
	//here we go: fetch, decode, execute
	while(running)
	{
        //fetch 
		cpu->IR = memRead(mem, cpu->PC);
        
        //decode, then execute
		switch(cpu->IR)
		{
            //############################# TRANSFER INSTRUCTIONS #############################
			case TAX_IMPL:  //X <- A, 1 byte long
            {
                PREPTEST(TAX_IMPL);
            
                cpu->PC++;                           //target next opcode
                tax();                          //execute opcode      
                        
                TEST(TAX_IMPL);            
                break;
            }
            
            case TXA_IMPL:  //A <- X, 1 byte long
            {
                PREPTEST(TXA_IMPL);
            
                cpu->PC++;                           //target next opcode
                txa();                          //execute opcode      
            
                TEST(TXA_IMPL);
                break;  
            }
            
            case TAY_IMPL:  //Y <- A, 1 byte long
            {
                PREPTEST(TAY_IMPL);
            
                cpu->PC++;                           //target next opcode
                tay();                          //execute opcode
            
                TEST(TAY_IMPL);
                break;  
            }
            
            case TYA_IMPL:  //A <- Y, 1 byte long
            {
                PREPTEST(TYA_IMPL);
            
                cpu->PC++;                           //target next opcode
                tya();                          //execute opcode      
            
                TEST(TYA_IMPL);            
                break;  
            }
                
            case TSX_IMPL:  //X <- SP, 1 byte long
            {
                PREPTEST(TSX_IMPL);
            
                cpu->PC++;                           //target next opcode
                tsx();                          //execute opcode      
            
                TEST(TSX_IMPL);
                break;  
            }
             
            case TXS_IMPL:  //SP <- X, 1 byte long
            {
                PREPTEST(TXS_IMPL);
            
                cpu->PC++;                           //target next opcode
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
                cpu->PC += 2;                        //target next opcode 
                lda(operand);                   //execute opcode
            
                TEST(LDA_IMMD);
                break;  
            }                
            case LDA_ZRP: //A <- M from zeropage, 2 bytes long
            {
                PREPTEST(LDA_ZRP);
            
                word operand = getZrpOp();      //get operand from zeropage
                cpu->PC += 2;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ZRP);
                break;  
            }         
            //TODO/TOCHECK/LATER
            case LDA_ZRPX: //A <- M from zeropage+X, 2 bytes long
            {
                PREPTEST(LDA_ZRPX);
            
                word operand = getZrpXOp();     //get operand from zeropage 
                cpu->PC += 2;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ZRPX);
                break;  
            }
            case LDA_ABS: //A <- M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(LDA_ABS);
            
                word operand = getAbsOp();      //get operand from absolute address  
                cpu->PC += 3;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ABS);
                break;                          
            }
            
            case LDA_ABSX: //A <- M from [[PChi,PClo]+X], 3 bytes long
            {
                PREPTEST(LDA_ABSX);
                
                word operand = getAbsXOp();     //get operand from absolute address + X
                cpu->PC += 3;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ABSX);
                break;                          
            }
            //TODO/TOCHECK/LATER
            case LDA_ABSY: //A <- M from [[PChi,PClo]+Y], 3 bytes long
            {
                PREPTEST(LDA_ABSY);
            
                word operand = getAbsYOp();     //get operand from absolute address + Y
                cpu->PC += 3;                        //target next opcode
                lda(operand);                   //execute opcode
            
                TEST(LDA_ABSY);
                break;                          
            }
            //TODO/TOCHECK/LATER
            case LDA_XIND: //A <- M [TODO], 2 bytes long
            {
                PREPTEST(LDA_XIND);

                word operand = getXIndOp();     //get X indexed indirect operand
                cpu->PC += 2;                        //target next opcode
                lda(operand);                   //execute opcode

                TEST(LDA_XIND);
                break;
            }
            //TODO/TOCHECK/LATER
            case LDA_INDY: //A <- M [TODO], 2 bytes long
            {
                word operand = getIndYOp();
                cpu->PC += 2;
                lda(operand);
                break;
            }
            
            //************ LDA: X <- M *************
            case LDX_IMMD: //X <- M, e.g. LDX #$FF ; load X with $FF, 2 bytes long
            {
                PREPTEST(LDX_IMMD);
            
                word operand = getImdOp();      //target operand 
                cpu->PC += 2;                        //target next opcode
                ldx(operand); 
            
                TEST(LDX_IMMD);
                break;  
            }
                
            case LDX_ZRP: //X <- M from zeropage, 2 bytes long
            {
                PREPTEST(LDX_ZRP);
            
                word operand = getZrpOp();      //get operand from zeropage 
                cpu->PC += 2;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ZRP);
                break;
            }
            
                
            case LDX_ZRPY: //X <- M from zeropage+Y, 2 bytes long
            {
                PREPTEST(LDX_ZRPY);
            
                word operand = getZrpYOp();     //get operand from zeropage 
                cpu->PC += 2;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ZRPY);
                break; 
            }

            case LDX_ABS: //X <- M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(LDX_ABS);
            
                word operand = getAbsOp();      //get operand from absolute address  
                cpu->PC += 3;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ABS);
                break;         
            }
            
            case LDX_ABSY: //X <- M from [[PChi,PClo]+Y], 3 bytes long
            {
                PREPTEST(LDX_ABSY);
            
                word operand = getAbsYOp();     //get operand from absolute address + Y
                cpu->PC += 3;                        //target next opcode
                ldx(operand);                   //execute opcode
            
                TEST(LDX_ABSY);
                break; 
            }
             
            //************ LDY: Y <- M *************
            case LDY_IMMD: //X <- M, e.g. LDX #$FF ; load X with $FF, 2 bytes long
            {
                PREPTEST(LDY_IMMD);
            
                word operand = getImdOp();      //target operand 
                cpu->PC += 2;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_IMMD);
                break;  
            }

            case LDY_ZRP: //X <- M from zeropage, 2 bytes long
            {
                PREPTEST(LDY_ZRP);
            
                word operand = getZrpOp();      //get operand from zeropage 
                cpu->PC += 2;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ZRP);
                break;
            }
            
            case LDY_ZRPX: //X <- M from zeropage+X, 2 bytes long
            {
                PREPTEST(LDY_ZRPX);
            
                word operand = getZrpXOp();     //get operand from zeropage 
                cpu->PC += 2;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ZRPX);
                break; 
            }
            
            case LDY_ABS: //X <- M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(LDY_ABS);
            
                word operand = getAbsOp();      //get operand from absolute address  
                cpu->PC += 3;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ABS);
                break;         
            }
            
            case LDY_ABSX: //X <- M from [[PChi,PClo]+X], 3 bytes long
            {
                PREPTEST(LDY_ABSX);
            
                word operand = getAbsXOp();     //get operand from absolute address + X
                cpu->PC += 3;                        //target next opcode
                ldy(operand);                   //execute opcode
            
                TEST(LDY_ABSX);
                break; 
            }
                
                
            //************ STA: A -> M *************
            case STA_ZRP: //A -> M from zeropage, 2 bytes long
            {               
                PREPTEST(STA_ZRP);
            
                address a = getZrpAddr();       //get address from zeropage
                cpu->PC += 2;                        //target next opcode
                sta(a);                         //execute opcode
            
                TEST(STA_ZRP);
                break;  
            }    
            
            case STA_ZRPX: //A -> M from zeropage+X, 2 bytes long
            {
                PREPTEST(STA_ZRPX);
            
                address a = getZrpXAddr();      //get address from zeropage + X
                cpu->PC += 2;                        //target next opcode
                sta(a);                         //execute opcode
            
                TEST(STA_ZRPX);
                break;  
            }       

            case STA_ABS: //A -> M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(STA_ABS);
            
                address a = getAbsAddr();       //get absolute address
                cpu->PC += 3;                        //target next opcode
                sta(a);                         //execute opcode
            
                TEST(STA_ABS);
                break;                          
            }
            
            case STA_ABSX: //A -> M from [PChi,PClo] + X, 3 bytes long
            {
                PREPTEST(STA_ABSX);
            
                address a = getAbsXAddr();      //get absolute address + X
                cpu->PC += 3;                        //target next opcode
                sta(a);                         //execute opcode
                
                TEST(STA_ABSX);
                break;                          
            }
            
            case STA_ABSY: //A -> M from [PChi,PClo] + Y, 3 bytes long
            {
                PREPTEST(STA_ABSY);
            
                address a = getAbsYAddr();      //get absolute address + Y
                cpu->PC += 3;                        //target next opcode
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
                cpu->PC += 2;                        //target next opcode
                stx(a);                         //execute opcode
                
                TEST(STX_ZRP);
                break;  
            } 
                
            //************ STX: X -> M *************
            case STX_ZRPY: //X -> M from zeropage+Y, 2 bytes long
            {               
                PREPTEST(STX_ZRPY);
                
                address a = getZrpYAddr();      //get address from zeropage + Y
                cpu->PC += 2;                        //target next opcode
                stx(a);                         //execute opcode
                
                TEST(STX_ZRPY);
                break;  
            }
                                
            case STX_ABS: //X -> M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(STX_ABS);
                
                address a = getAbsAddr();       //get absolute address
                cpu->PC += 3;                        //target next opcode
                stx(a);                         //execute opcode
                
                TEST(STX_ABS);
                break;                          
            }
                
              
            //************ STY: Y -> M *************                
            case STY_ZRP: //Y -> M from zeropage, 2 bytes long
            {               
                PREPTEST(STY_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage
                cpu->PC += 2;                        //target next opcode
                sty(a);                         //execute opcode
                
                TEST(STY_ZRP);
                break;  
            } 
                
            //************ STY: Y -> M *************
            case STY_ZRPX: //Y -> M from zeropage+X, 2 bytes long
            {               
                PREPTEST(STY_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage + X
                cpu->PC += 2;                        //target next opcode
                sty(a);                         //execute opcode
                
                TEST(STY_ZRPX);
                break;  
            }
                
                
            case STY_ABS: //Y -> M from [PChi,PClo], 3 bytes long
            {
                PREPTEST(STY_ABS);
                
                address a = getAbsAddr();       //get absolute address
                cpu->PC += 3;                        //target next opcode
                sty(a);                         //execute opcode
                
                TEST(STY_ABS);
                break;                          
            }

            
                
                
                
            //############################# ARITHMETIC INSTRUCTIONS #############################
            //ADC:  Add Memory to Accumulator with Carry: A <- A + M + C
            case ADC_IMMD: //2 bytes long
            {
                word operand = getImdOp();      //target operand 
                cpu->PC += 2;                        //target next opcode
                adc(operand);                   //execute opcode
                break;                   
            }
            case ADC_ZRP: //2 bytes long
            {
                word operand = getZrpOp();      //get operand from zeropage
                cpu->PC += 2;                        //target next opcode
                adc(operand);                   //execute opcode
                break;                   
            }
            case ADC_ABS: //2 bytes long
            {
                word operand = getAbsOp();      //get operand from absolute address
                cpu->PC += 2;                        //target next opcode
                adc(operand);                   //execute opcode
                break;                   
            }
                
            //SBC: Subtract Memory from Accumulator with Borrow: A - M - !C -> A
            case SBC_IMMD: //2 bytes long
            {
                word operand = getImdOp();      //target operand 
                cpu->PC += 2;                        //target next opcode
                sbc(operand);                   //execute opcode
                break;
            }
                
            //TODO: other SBCs here
            
                
            case INC_ZRP: //2 bytes long
            {
                PREPTEST(INC_ZRP);

                address a = getZrpAddr();       //get address from zeropage                 
                cpu->PC += 2;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ZRP);
                break;
            }
            
            case INC_ZRPX: //2 bytes long
            {
                PREPTEST(INC_ZRPX);

                address a = getZrpXAddr();      //get address from zeropage + X                 
                cpu->PC += 2;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ZRPX);
                break;
            }

            case INC_ABS: //3 bytes long
            {
                PREPTEST(INC_ABS);

                address a = getAbsAddr();       //get absolute address                
                cpu->PC += 3;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ABS);
                break;
            }

            case INC_ABSX: //3 bytes long
            {
                PREPTEST(INC_ABSX);

                address a = getAbsXAddr();      //get absolute address                
                cpu->PC += 3;                        //target next opcode
                inc(a);                         //execute opcode

                TEST(INC_ABSX);
                break;
            }
            
            
            case INX_IMPL: 
            {
                PREPTEST(INX_IMPL);
            
                cpu->PC++;                //target next opcode
                inx();               //execute opcode
            
                TEST(INX_IMPL);
                break;        
            }
                
            case INY_IMPL: 
            {
                PREPTEST(INY_IMPL);
            
                cpu->PC++;                //target next opcode
                iny();               //execute opcode
            
                TEST(INY_IMPL);
                break;        
            }

            case DEC_ZRP:
            {
                PREPTEST(DEC_ZRP);

                address a = getZrpAddr();       //get address from zeropage                 
                cpu->PC += 2;                        //target next opcode
                dec(a);                         //execute opcode

                TEST(DEC_ZRP);
                break;
            }
            
            case DEC_ZRPX:
            {
                PREPTEST(DEC_ZRPX);

                address a = getZrpXAddr();      //get address from zeropage + X                 
                cpu->PC += 2;                        //target next opcode
                dec(a);                         //execute opcode

                TEST(DEC_ZRPX);
                break;
            }

            case DEC_ABS:
            {
                PREPTEST(DEC_ABS);

                address a = getAbsAddr();       //get absolute address                
                cpu->PC += 3;                        //target next opcode
                dec(a);                         //execute opcode

                TEST(DEC_ABS);
                break;
            }

            case DEC_ABSX:
            {
                PREPTEST(DEC_ABSX);

                address a = getAbsXAddr();      //get absolute address                
                cpu->PC += 3;                        //target next opcode
                dec(a);                         //execute opcode

                TEST(DEC_ABSX);
                break;
            }
                
            case DEX_IMPL: 
            {
                PREPTEST(DEX_IMPL);
            
                cpu->PC++;                //target next opcode
                dex();               //execute opcode
            
                TEST(DEX_IMPL);
                break;        
            }
                
            case DEY_IMPL: 
            {
                PREPTEST(DEY_IMPL);
            
                cpu->PC++;                //target next opcode
                dey();               //execute opcode
            
                TEST(DEY_IMPL);
                break;        
            }
                
                
            //############################# SHIFT & ROTATE INSTRUCTIONS #############################
            case ASL_ACCU: 
            {
                PREPTEST(ASL_ACCU);
            
                cpu->PC++;                //target next opcode
                asl_accu();          //execute opcode
            
                TEST(ASL_ACCU);
                break;        
            }

            case ASL_ZRP: 
            {
                PREPTEST(ASL_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                cpu->PC += 2;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ZRP);
                break;        
            }
            
            case ASL_ZRPX: 
            {
                PREPTEST(ASL_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                cpu->PC += 2;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ZRPX);
                break;        
            }

            case ASL_ABS: 
            {
                PREPTEST(ASL_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                cpu->PC += 3;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ABS);
                break;        
            }

            case ASL_ABSX: 
            {
                PREPTEST(ASL_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                cpu->PC += 3;                        //target next opcode                
                asl(a);                         //execute opcode

                TEST(ASL_ABSX);
                break;        
            }

            case LSR_ACCU: 
            {
                PREPTEST(LSR_ACCU);
            
                cpu->PC++;                           //target next opcode
                lsr_accu();                     //execute opcode
            
                TEST(LSR_ACCU);
                break;        
            }

            case LSR_ZRP: 
            {
                PREPTEST(LSR_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                cpu->PC += 2;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ZRP);
                break;        
            }
            
            case LSR_ZRPX: 
            {
                PREPTEST(LSR_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                cpu->PC += 2;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ZRPX);
                break;        
            }

            case LSR_ABS: 
            {
                PREPTEST(LSR_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                cpu->PC += 3;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ABS);
                break;        
            }

            case LSR_ABSX: 
            {
                PREPTEST(LSR_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                cpu->PC += 3;                        //target next opcode                
                lsr(a);                         //execute opcode

                TEST(LSR_ABSX);
                break;        
            }

            case ROL_ACCU: 
            {
                PREPTEST(ROL_ACCU);
            
                cpu->PC++;                //target next opcode
                rol_accu();          //execute opcode
            
                TEST(ROL_ACCU);
                break;        
            }

            case ROL_ZRP: 
            {
                PREPTEST(ROL_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                cpu->PC += 2;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ZRP);
                break;        
            }
            
            case ROL_ZRPX: 
            {
                PREPTEST(ROL_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                cpu->PC += 2;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ZRPX);
                break;        
            }

            case ROL_ABS: 
            {
                PREPTEST(ROL_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                cpu->PC += 3;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ABS);
                break;        
            }

            case ROL_ABSX: 
            {
                PREPTEST(ROL_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                cpu->PC += 3;                        //target next opcode                
                rol(a);                         //execute opcode

                TEST(ROL_ABSX);
                break;        
            }
            
                
            case ROR_ACCU: 
            {
                PREPTEST(ROR_ACCU);
            
                cpu->PC++;                //target next opcode
                ror_accu();          //execute opcode
            
                TEST(ROR_ACCU);
                break;        
            }

            case ROR_ZRP: 
            {
                PREPTEST(ROR_ZRP);
                
                address a = getZrpAddr();       //get address from zeropage                 
                cpu->PC += 2;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ZRP);
                break;        
            }
            
            case ROR_ZRPX: 
            {
                PREPTEST(ROR_ZRPX);
                
                address a = getZrpXAddr();      //get address from zeropage+X                 
                cpu->PC += 2;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ZRPX);
                break;        
            }

            case ROR_ABS: 
            {
                PREPTEST(ROR_ABS);
                
                address a = getAbsAddr();       //get absoulte address                 
                cpu->PC += 3;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ABS);
                break;        
            }

            case ROR_ABSX: 
            {
                PREPTEST(ROR_ABSX);
                
                address a = getAbsXAddr();      //get absoulte+X address                 
                cpu->PC += 3;                        //target next opcode                
                ror(a);                         //execute opcode

                TEST(ROR_ABSX);
                break;        
            }

            //############################# LOGIC INSTRUCTIONS #############################
            case AND_IMMD: 
            {
                PREPTEST(AND_IMMD);
                
                word operand = getImdOp();      //get immediate operand
                cpu->PC += 2;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_IMMD);
                break;        
            }

            case AND_ZRP: 
            {
                PREPTEST(AND_ZRP);
                
                word operand = getZrpOp();      //get zeropage operand
                cpu->PC += 2;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ZRP);
                break;        
            }

            case AND_ZRPX: 
            {
                PREPTEST(AND_ZRPX);
                
                word operand = getZrpXOp();     //get zeropage+X operand
                cpu->PC += 2;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ZRPX);
                break;        
            }

            case AND_ABS: 
            {
                PREPTEST(AND_ABS);
                
                word operand =  getAbsOp();     //get absoulte operand
                cpu->PC += 3;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ABS);
                break;        
            }

            case AND_ABSX: 
            {
                PREPTEST(AND_ABSX);
                
                word operand =  getAbsXOp();    //get absolute+X operand
                cpu->PC += 3;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ABSX);
                break;        
            }

            case AND_ABSY: 
            {
                PREPTEST(AND_ABSY);
                
                word operand =  getAbsYOp();    //get absolute+Y operand
                cpu->PC += 3;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_ABSY);
                break;        
            }

            case AND_XIND: 
            {
                PREPTEST(AND_XIND);
                
                word operand =  getXIndOp();    //get X indexed indirect operand
                cpu->PC += 2;                        //target next opcode
                and(operand);                   //execute opcode
                
                TEST(AND_XIND);
                break;        
            }

            case AND_INDY: 
            {
             //TODO      
             break;;  
            }

            case ORA_IMMD: 
            {
                PREPTEST(ORA_IMMD);
                
                word operand = getImdOp();      //get immediate operand
                cpu->PC += 2;                        //target next opcode
                ora(operand);                   //execute opcode
                
                TEST(ORA_IMMD);
                break;        
            }

            case ORA_ZRP: 
            {
                PREPTEST(ORA_ZRP);
                
                word operand = getZrpOp();      //get zeropage operand
                cpu->PC += 2;                        //target next opcode
                ora(operand);                   //execute opcode
                
                TEST(ORA_ZRP);
                break;        
            }

            case ORA_ZRPX: 
            {
                PREPTEST(ORA_ZRPX);
                
                word operand = getZrpXOp();     //get zeropage+X operand
                cpu->PC += 2;                        //target next opcode
                ora(operand);                   //execute opcode
                
                TEST(ORA_ZRPX);
                break;        
            }

            case ORA_ABS: 
            {
                PREPTEST(ORA_ABS);
                
                word operand =  getAbsOp();     //get absoulte operand
                cpu->PC += 3;                        //target next opcode
                ora(operand);                   //execute opcode
                
                TEST(ORA_ABS);
                break;        
            }

            case ORA_ABSX: 
            {
                PREPTEST(ORA_ABSX);
                
                word operand =  getAbsXOp();    //get absolute+X operand
                cpu->PC += 3;                        //target next opcode
                ora(operand);                   //execute opcode
                
                TEST(ORA_ABSX);
                break;        
            }

            case ORA_ABSY: 
            {
                PREPTEST(ORA_ABSY);
                
                word operand =  getAbsYOp();    //get absolute+Y operand
                cpu->PC += 3;                        //target next opcode
                ora(operand);                   //execute opcode
                
                TEST(ORA_ABSY);
                break;        
            }

            case ORA_XIND: 
            {
                PREPTEST(ORA_XIND);
                
                word operand =  getXIndOp();    //get X indexed indirect operand
                cpu->PC += 2;                        //target next opcode
                ora(operand);                   //execute opcode
                
                TEST(ORA_XIND);
                break;        
            }

            case ORA_INDY: 
            {
             //TODO      
             break;;  
            }


            case EOR_IMMD: 
            {
                PREPTEST(EOR_IMMD);
                
                word operand = getImdOp();      //get immediate operand
                cpu->PC += 2;                        //target next opcode
                eor(operand);                   //execute opcode
                
                TEST(EOR_IMMD);
                break;        
            }

            case EOR_ZRP: 
            {
                PREPTEST(EOR_ZRP);
                
                word operand = getZrpOp();      //get zeropage operand
                cpu->PC += 2;                        //target next opcode
                eor(operand);                   //execute opcode
                
                TEST(EOR_ZRP);
                break;        
            }

            case EOR_ZRPX: 
            {
                PREPTEST(EOR_ZRPX);
                
                word operand = getZrpXOp();     //get zeropage+X operand
                cpu->PC += 2;                        //target next opcode
                eor(operand);                   //execute opcode
                
                TEST(EOR_ZRPX);
                break;        
            }

            case EOR_ABS: 
            {
                PREPTEST(EOR_ABS);
                
                word operand =  getAbsOp();     //get absoulte operand
                cpu->PC += 3;                        //target next opcode
                eor(operand);                   //execute opcode
                
                TEST(EOR_ABS);
                break;        
            }

            case EOR_ABSX: 
            {
                PREPTEST(EOR_ABSX);
                
                word operand =  getAbsXOp();    //get absolute+X operand
                cpu->PC += 3;                        //target next opcode
                eor(operand);                   //execute opcode
                
                TEST(EOR_ABSX);
                break;        
            }

            case EOR_ABSY: 
            {
                PREPTEST(EOR_ABSY);
                
                word operand =  getAbsYOp();    //get absolute+Y operand
                cpu->PC += 3;                        //target next opcode
                eor(operand);                   //execute opcode
                
                TEST(EOR_ABSY);
                break;        
            }

            case EOR_XIND: 
            {
                PREPTEST(EOR_XIND);
                
                word operand =  getXIndOp();    //get X indexed indirect operand
                cpu->PC += 2;                        //target next opcode
                eor(operand);                   //execute opcode
                
                TEST(EOR_XIND);
                break;        
            }

            case EOR_INDY: 
            {
             //TODO      
             break;;  
            }



            //############################# COMPARE AND TEST BIT INSTRUCTIONS #############################
            case CMP_IMMD: 
            {
                PREPTEST(CMP_IMMD);
                
                word operand = getImdOp();      //get immediate operand
                cpu->PC += 2;                        //target next opcode
                cmp(operand);                   //execute opcode
                
                TEST(CMP_IMMD);
                break;

            }
                
            //############################# SET AND CLEAR INSTRUCTIONS #############################
            case SEC_IMPL: 
            {
                PREPTEST(SEC_IMPL);
            
                cpu->PC++;   //target next opcode
                sec();  //execute opcode
            
                TEST(SEC_IMPL);
                break; 
            }
            
            case SED_IMPL: 
            {
                PREPTEST(SED_IMPL);
            
                cpu->PC++;   //target next opcode
                sed();  //execute opcode
            
                TEST(SED_IMPL);
                break; 
            }
                
            case SEI_IMPL: 
            {
                PREPTEST(SEI_IMPL);
            
                cpu->PC++;   //target next opcode
                sei();  //execute opcode
            
                TEST(SEI_IMPL);
                break; 
            }
                
                
                
            case CLC_IMPL: 
            {
                PREPTEST(CLC_IMPL);
            
                cpu->PC++;   //target next opcode
                clc();  //execute opcode
                
                TEST(CLC_IMPL);
                break; 
            }
                
            case CLD_IMPL: 
            {
                PREPTEST(CLD_IMPL);
            
                cpu->PC++;   //target next opcode
                cld();  //execute opcode
            
                TEST(CLD_IMPL);
                break; 
            }
                
            case CLI_IMPL: 
            {
                PREPTEST(CLI_IMPL);
            
                cpu->PC++;   //target next opcode
                cli();  //execute opcode
            
                TEST(CLI_IMPL);
                break; 
            }
                
            case CLV_IMPL: 
            {
                PREPTEST(CLV_IMPL);
            
                cpu->PC++;   //target next opcode
                clv();  //execute opcode
            
                TEST(CLV_IMPL);
                break; 
            }
                
            //############################# JUMP AND SUBROUTINE INSTRUCTIONS #############################
            case JMP_ABS:
            {
                PREPTEST(JMP_ABS);
            
                address a = getAbsAddr();   //get absolute address
                cpu->PC += 3;                    //target next opcode
                jmp(a);                     //execute opcode
            
                TEST(JMP_ABS);
                break; 
            }

            case JMP_IND: //TODO
            {
                PREPTEST(JMP_IND);
            
                // cpu->PC++;   //target next opcode
                // jmp();  //execute opcode
            
                TEST(JMP_IND);
                break; 
            }

            case JSR_ABS:
            {
                PREPTEST(JSR_ABS);

                address a = getAbsAddr();       //get absoulte address
                cpu->PC += 2;                        //it's 3-byte opcode but we must increment only by 2 (corresponding RTS will increment the PC later)
                jsr(a);                         //execute opcode
            
                TEST(JSR_ABS);
                break; 
            }

            case RTS_IMPL:
            {
                PREPTEST(RTS_IMPL);
                            
                rts();  //execute opcode
                cpu->PC++;   //PC must be incremented after pulling it from stack 
                        //because it still points to JSR's 2nd parameter rather than to the next opcode
            
                TEST(RTS_IMPL);
                break; 
            }

            case RTI_IMPL: //TODO
            {
                PREPTEST(RTI_IMPL);
            
                // cpu->PC++;   //target next opcode
                // jsr();  //execute opcode
            
                TEST(RTI_IMPL);
                break; 
            }                      
                            
            //############################# BRANCH INSTRUCTIONS #############################

            case BCC_REL:
            {
                PREPTEST(BCC_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                bcc(operand);               //execute opcode

                TEST(BCC_REL);
                break;
            }

            case BCS_REL:
            {
                PREPTEST(BCS_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                bcs(operand);               //execute opcode

                TEST(BCS_REL);
                break;
            }

            case BEQ_REL:
            {
                PREPTEST(BEQ_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                beq(operand);               //execute opcode

                TEST(BEQ_REL);
                break;
            }

            case BMI_REL:
            {
                PREPTEST(BMI_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                bmi(operand);               //execute opcode

                TEST(BMI_REL);
                break;
            }

            case BNE_REL: 
            {
                PREPTEST(BNE_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                bne(operand);               //execute opcode

                TEST(BNE_REL);
                break;
            }

            case BPL_REL: 
            {
                PREPTEST(BPL_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                bpl(operand);               //execute opcode

                TEST(BPL_REL);
                break;
            }

            case BVC_REL: 
            {
                PREPTEST(BVC_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                bvc(operand);               //execute opcode

                TEST(BVC_REL);
                break;
            }

            case BVS_REL: 
            {
                PREPTEST(BVS_REL);

                sword operand = getRelOp(); //get relative operand, which is a signed word
                cpu->PC += 2;                    //target next instruction, if branch is taken, we'll jump from here
                bvs(operand);               //execute opcode

                TEST(BVS_REL);
                break;
            }
                
            //############################# STACK INSTRUCTIONS #############################
            case PHA_IMPL:
            {
                PREPTEST(PHA_IMPL);

                cpu->PC++;
                pha();

                TEST(PHA_IMPL);
                break;           
            }    

            case PLA_IMPL:
            {
                PREPTEST(PLA_IMPL);

                cpu->PC++;
                pla();

                TEST(PLA_IMPL);
                break;           
            } 

            case PHP_IMPL:
            {
                PREPTEST(PHP_IMPL);

                cpu->PC++;
                php();

                TEST(PHP_IMPL);
                break;           
            } 

            case PLP_IMPL:
            {
                PREPTEST(PLP_IMPL);

                cpu->PC++;
                plp();

                TEST(PLP_IMPL);
                break;           
            } 


            //############################# MISC INSTRUCTIONS #############################
            case NOP_IMPL: //do nothing, 1 byte long
            {
                cpu->PC++; //nothing to do, just target next operand
                break;
            }

            //case BRK_IMPL:
            //{
                //cpu->PC++;
                //brk(); //TODO
                //printf("TODO: BRK NOT IMPLEMENTED");
            //    break;
            //}

            //TODO: delte this if BRK works properly, BKR is also 0x0
            case 0x0: //no instruction found, assuming that program has exited
            {
                printf("\nNo more instructions. Emulation stopped. \n");
                running = 0;
                break;
            }
            
			default: //invalid instruction
            { 
                printf("\nError: unknown instruction: 0x%X at 0x%.4X. Emulation stopped. \n", cpu->IR, cpu->PC); 
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
    cpu->X = cpu->A;
                
    //set flags
    setNByWord(cpu->X);
    setZByWord(cpu->X); 
}

//A <- X
//affects N and Z
void txa(void)
{
    cpu->A = cpu->X;    
    
    //set flags
    setNByWord(cpu->A); 
    setZByWord(cpu->A); 
}


//Y <- A
//affects N and Z
void tay(void)
{
    cpu->Y = cpu->A;       
    
    //set flags
    setNByWord(cpu->Y); 
    setZByWord(cpu->Y);
}

//A <- Y
//affects N and Z
void tya(void)
{
    cpu->A = cpu->Y;      

    //set flags
    setNByWord(cpu->A); 
    setZByWord(cpu->A);    
}

//X <- SP
//affects N and Z
void tsx(void)
{    
	cpu->X = cpu->SP;       
                
    //set flags
    setNByWord(cpu->X); 
    setZByWord(cpu->X);    
}

//SP <- X
//no flags
void txs(void)
{
    cpu->SP = cpu->X;      
}


//A <- M
//affects N and Z
void lda(word operand)
{      
    cpu->A = operand; 
                
    //set flags
    setNByWord(cpu->A);
    setZByWord(cpu->A);    
}

//A -> M
//no flags
void sta(address a)
{      
    memWrite(mem, cpu->A, a); //write contents of A to address a   
}

//X -> M
void stx(address a)
{      
    memWrite(mem, cpu->X, a); //write contents of X to address a    
}

//Y -> M
void sty(address a)
{      
    memWrite(mem, cpu->Y, a); //write contents of Y to address a    
}


//############################# ARITHMETIC INSTRUCTIONS #############################
//A <- A + M + C
//affects N, V, Z and C 
//note: decimal mode is not treated here, since NES' 6502 lacks BCD mode
void adc(word operand)
{
    word Ainit = cpu->A;                         //get initial value of A since we need it to do some checks with it later
    
    dword A16 = Ainit + operand + getC();   //store result in 16 bit int to check whether it is greater than 8 bit
    cpu->A = A16 & 0x0000FFFF;                   //copy result without cary (if exists) to A
    
    setNByWord(cpu->A);
    
    //if +a + +b got -c or -a + -b got +c then we have an overflow here (result didn't fit into 8 bit and wrapped over)
    if ( (isN(operand) && isN(Ainit) && !isN(cpu->A)) || (!isN(operand) && !isN(Ainit) && isN(cpu->A)) )
    {
        setVByFlag(1);
    }
    
    setZByWord(cpu->A); 
    
    //result > 255 => 8 bits were not sufficient => need 9th bit = carry, otherwise clear carry, which might be set (and used) before
    (A16 > 0xFF) ? setCByFlag(1) : setCByFlag(0); 
    
       
    //TODO: test with http://skilldrick.github.io/easy6502/    
    printRegs(cpu);
}

//SBC: Subtract Memory from Accumulator with Borrow: A - M - !C -> A
//affects N, V, Z and C 
void sbc(word operand)
{
    word Ainit = cpu->A;                         //get initial value of A since we need it to do some checks with it later
    
    dword A16 = Ainit - operand - !getC();  //store result in 16 bit int to check whether it is greater than 8 bit
    cpu->A = A16 & 0x0000FFFF;                   //copy result without cary (if exists) to A
    
    setNByWord(cpu->A);
    
    //TODO: do stuff here too
    
    //TODO: test with http://skilldrick.github.io/easy6502/    
    printRegs(cpu);
    
}

//increment memory: M <- M + 1
//affects N and Z
void inc(address a)
{
    word w = memRead(mem, a);
    memWrite(mem, ++w, a);

    setNByWord(w);
    setZByWord(w);
}



//increment X
//affects N and Z
void inx(void)
{
    cpu->X++;
    
    setNByWord(cpu->X);
    setZByWord(cpu->X);        
}

//increment Y
//affects N and Z
void iny(void)
{
    cpu->Y++;
    
    setNByWord(cpu->Y);
    setZByWord(cpu->Y);
}

//decrement memory at address a
//affects N and Z
void dec(address a)
{
    word w = memRead(mem, a); //get value from mem
    w--;             //decrement it
    memWrite(mem, w, a);       //write it back

    setNByWord(w);
    setZByWord(w);
}


//decrement X
//affects N and Z
void dex(void)
{
    cpu->X--;
    
    setNByWord(cpu->X);
    setZByWord(cpu->X);        
}

//decrement Y
//affects N and Z
void dey(void)
{
    cpu->Y--;
    
    setNByWord(cpu->Y);
    setZByWord(cpu->Y);        
}

//A <- (A << 1), original bit #7 is stored to carry flag
//affects N, Z, C
void asl_accu(void)
{   
    setCByFlag(getBit(cpu->A, 7)); //before shifting, save bit #7 to carry
    
    cpu->A = cpu->A << 1; //the actual shift operation   

    setNByWord(cpu->A);
    setZByWord(cpu->A); 
}

//M[a] <- (M[a] << 1), original bit #7 is stored to carry flag
//affects N, Z, C
void asl(address a)
{   
    word w = memRead(mem, a); //get word stored at address

    setCByFlag(getBit(w, 7)); //before shifting, save bit #7 to carry
    
    w = w << 1; //the actual shift operation

    memWrite(mem, w, a); //write back updated word

    setNByWord(w);
    setZByWord(w); 
}

//A <- (A >> 1), original bit #0 is stored to carry flag
//affects N, Z, C
void lsr_accu(void)
{   
    setCByFlag(getBit(cpu->A, 0)); //before shifting, save bit #0 to carry
    
    cpu->A = cpu->A >> 1; //the actual shift operation   

    setNByWord(cpu->A);
    setZByWord(cpu->A); 
}

//M[a] <- (M[a] >> 1), original bit #0 is stored to carry flag
//affects N, Z, C
void lsr(address a)
{   
    word w = memRead(mem, a); //get word stored at address

    setCByFlag(getBit(w, 0)); //before shifting, save bit #0 to carry
    
    w = w >> 1; //the actual shift operation

    memWrite(mem, w, a); //write back updated word

    setNByWord(w);
    setZByWord(w); 
}

//rotate left: shift A left, copy original bit #7 to carry and to bit #0 of A
//affects N, Z, C
void rol_accu(void)
{   
    setCByFlag(getBit(cpu->A, 7)); //before shifting, save bit #7 to carry
    
    cpu->A = cpu->A << 1; //the actual shift operation

    cpu->A = cpu->A | getC(); //copy carry to bit #0

    setNByWord(cpu->A);
    setZByWord(cpu->A); 
}

//rotate left: shift M[a] left, copy original bit #7 to carry and to bit #0 of M[a]
//affects N, Z, C
void rol(address a)
{   
    word w = memRead(mem, a); //get word stored at address

    setCByFlag(getBit(w, 7)); //before shifting, save bit #7 to carry
    
    w = w << 1; //the actual shift operation

    w = w | getC(); //copy carry to bit #0

    memWrite(mem, w, a); //write back updated word

    setNByWord(w);
    setZByWord(w); 
}

//rotate right: shift A right, copy original bit #0 to carry and to bit #7 of A
//affects N, Z, C
void ror_accu(void)
{   
    setCByFlag(getBit(cpu->A, 0)); //before shifting, save bit #0 to carry
    
    cpu->A = cpu->A >> 1; //the actual shift operation

    cpu->A = cpu->A | (getC() << 7); //copy carry to bit #7

    setNByWord(cpu->A);
    setZByWord(cpu->A); 
}

//rotate right: shift M[a] rigth, copy original bit #0 to carry and to bit #7 of M[a]
//affects N, Z, C
void ror(address a)
{   
    word w = memRead(mem, a); //get word stored at address

    setCByFlag(getBit(w, 0)); //before shifting, save bit #0 to carry
    
    w = w >> 1; //the actual shift operation

    w = w | (getC() << 7); //copy carry to bit #7

    memWrite(mem, w, a); //write back updated word

    setNByWord(w);
    setZByWord(w); 
}

//A <-- A & operand
//affects N, Z
void and(word operand)
{
    cpu->A = cpu->A & operand;

    setNByWord(cpu->A);
    setZByWord(cpu->A);
}

//A <-- A | operand
//affects N, Z
void ora(word operand)
{
    cpu->A = cpu->A | operand;

    setNByWord(cpu->A);
    setZByWord(cpu->A);
}

//A <-- A ^ operand
//affects N, Z
void eor(word operand)
{
    cpu->A = cpu->A ^ operand;

    setNByWord(cpu->A);
    setZByWord(cpu->A);
}

//A - M (compute difference = compare)
//affects N, Z, C
void cmp (word operand)
{
    int diff = (int)cpu->A - (int)operand;

    if (diff == 0)
    {
        setCByFlag(1);
        setZByFlag(1);
        return;
    }

    if (diff > 0)
    {
        setCByFlag(1);
        return;
    }

    if (diff < 0)
    {
        setNByFlag(1);
        return;
    }
}

                        
//C <-- 1
//affects C
void sec(void)
{
    setCByFlag(1); //set C
}

//D <-- 1
//affects D
void sed(void)
{
    setDByFlag(1); //set D
}

//I <-- 1
//affects I
void sei(void)
{
    setIByFlag(1); //set I
}


//C <-- 0
//affects C
void clc(void)
{
    setCByFlag(0); //clear C
}

//D <-- 0
//affects D
void cld(void)
{
    setDByFlag(0); //clear D
}

//I <-- 0
//affects I
void cli(void)
{
    setIByFlag(0); //clear I
}

//V <-- 0
//affects V
void clv(void)
{
    setVByFlag(0); //clear V
}

//jump to new location
//3 bytes long
//no flags affected
void jmp(address a)
{
    cpu->PC = a;
}



//jump to subroutine: push PC to stack and load PC with jump address a
//note: JSR instruction increments the PC only by 2 (according to real HW implementation)
//the PC is incremented to proper address later by corresponding RTS
//no flags affected
void jsr(address a)
{
    memWrite(mem, (cpu->PC & 0xFF00)>>2, cpu->SP);  //push PC-HI to stack

    cpu->SP--;                       //point to next free stack location

    memWrite(mem, cpu->PC & 0x00FF, cpu->SP);       //push PC-LO to stack

    cpu->SP--;                       //point to next free stack location
    
    cpu->PC = a;                     //store jump address to PC

    //pushing beyond MAX?
    warnStackOverflow("JSR", cpu->PC-2);
}

//return from subroutine: pull previously saved PC value from stack and loat it into PC register
//note: after pulling the PC from stack it must be incremented (according to real HW implementation)
//no flags affected
void rts(void)
{    
    cpu->SP++;                       //target value on stack that will be pulled

    word pclo = memRead(mem, cpu->SP);        //pull value from stack, the value should be LO byte of the previously pushed PC register
    
    cpu->SP++;                       //target next stack value that will be pulled
    
    word pchi = memRead(mem, cpu->SP);        //pull value from stack, the value should be HI byte of the previously pushed PC register

    cpu->PC = lohi2addr(pclo, pchi); //from LO byte and HI byte, construct address and store it into PC register (PC is then restored after JSR)

    //pulling beyond MIN?
    warnStackUnderflow("RTS", cpu->PC);
}


//X <- M
//affects N and Z
void ldx(word operand)
{
    cpu->X = operand;
    
    //set flags
    setNByWord(cpu->X);
    setZByWord(cpu->X);    
}

//Y <- M
//affects N and Z
void ldy(word operand)
{
    cpu->Y = operand;
    
    //set flags
    setNByWord(cpu->Y);
    setZByWord(cpu->Y);    
}

//branch to PC+operand if C is 0
//2 bytes long, no flags affected
void bcc(sword operand)
{    
    if (getC() == 0)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }
}

//branch to PC+operand if C is 1
//2 bytes long, no flags affected
void bcs(sword operand)
{    
    if (getC() == 1)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }
}

//branch to PC+operand if Z is 1
//2 bytes long, no flags affected
void beq(sword operand)
{    
    if (getZ() == 1)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }
}

//branch to PC+operand if N is 1
//2 bytes long, no flags affected
void bmi(sword operand)
{    
    if (getN() == 1)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }
}

//branch to PC+operand if Z is 0
//2 bytes long, no flags affected
void bne(sword operand)
{
    if (getZ() == 0)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }    
}

//branch to PC+operand if N is 0
//2 bytes long, no flags affected
void bpl(sword operand)
{
    if (getN() == 0)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }    
}

//branch to PC+operand if V is 0
//2 bytes long, no flags affected
void bvc(sword operand)
{
    if (getV() == 0)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }    
}

//branch to PC+operand if V is 1
//2 bytes long, no flags affected
void bvs(sword operand)
{
    if (getV() == 1)
    {
        //branch taken, add signed offset to jump to current position + offset, which is in [-128, 127]
        cpu->PC = (address) ((int) cpu->PC + operand);
    }    
}

//push A to stack, i.e. mem[SP] <- A
//no flags affected
void pha(void)
{
    memWrite(mem, cpu->A, cpu->SP);     //push A to stack
    cpu->SP--;           //point to next free stack location
    
    warnStackOverflow("PHA", cpu->PC-1); //pushing beyond MAX?
}

//pull value from stack into A, i.e. A <- mem[SP+1]
//affects N and Z
void pla(void)
{
    cpu->SP++;           //target value on top of the stack

    cpu->A = memRead(mem, cpu->SP);    //copy value from stack into A, value in SP is free to be overwritten by next push operation

    //set flags
    setNByWord(cpu->A);
    setZByWord(cpu->A); 

    warnStackUnderflow("PLP", cpu->PC-1); //pushing beyond MIN?
}

//push P to stack, i.e. mem[SP] <- P
//no flags affected
void php(void)
{
    memWrite(mem, cpu->P, cpu->SP); //push P to stack
    cpu->SP--;       //point to next free stack location

    warnStackOverflow("PHP", cpu->PC-1); //pushing beyond MAX?
}

//pull value from stack into P, i.e. P <- mem[SP+1]
//affects all bits in P, because a new value is fetched into P
void plp(void)
{
    cpu->SP++;        //target value on top of the stack

    cpu->P = memRead(mem, cpu->SP); //copy value from stack into P, value in cpu->cpu->cpu->SP is free to be overwritten by next push operation

    warnStackUnderflow("PLP", cpu->PC-1); //pushing beyond MIN?
}
                 


#endif