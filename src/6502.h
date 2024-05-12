#ifndef _6502_H
#define _6502_H


#include "types.h"
#include "mem.h"


typedef struct 
{
    word 	X;      //X indexing register
    word 	Y;      //Y indexing register
    word 	A;      //accumulator
    word 	P;      //processor status word with the flags (N V - B D I Z C)
    word 	IR;     //instruction register, contains instruction to be decoded, i.e. IR == mrd(PC)
    address SP;     //6502's stack has a range of 256 and is hard wired to 2nd memory page 0100 to 01FF (9 bit address)
    address PC;     //program counter, NOTE: PC contains always the instruction to be fetched next !!!
    TMemory mem;
} CpuStruct;

typedef CpuStruct* T6502; 

typedef enum 
{
    CPU_STEP_OK = 0, 
    CPU_STEP_ERROR = 1
} eCpuStepStatus;

T6502 cpuInit(TMemory mem);

//void cpuConnectMemory(TMemory _mem);

eCpuStepStatus cpuStep(T6502 cpu);

//TODO MOVE THE STUFF BELOW TO C FILE !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ???



//TRANSFER INSTRUCTIONS (single byte instructions, operand addr is implied by opcode)
#define TAX_IMPL    0xAA    //transfer A to X
#define TXA_IMPL    0x8A    //transfer X to A              
#define TAY_IMPL    0xA8    //transfer A to Y
#define TYA_IMPL    0x98    //transfer Y to A                
#define TSX_IMPL    0xBA    //(tansfer stack pointer to X) 
#define TXS_IMPL    0x9A    //(tansfer X to stack pointer) NOTE: Although many instructions modify the value of the Stack Pointer, TXS is the only way to set it to a specified value.             

//STORAGE INSTRUCTIONS
#define LDA_IMMD    0xA9	//done => 2test
#define LDA_ZRP     0xA5	//done => 2test
#define LDA_ZRPX    0xB5	//done => 2test, DOITLATER 
#define LDA_ABS     0xAD	//done => 2test
#define LDA_ABSX    0xBD	//done => 2test, DOITLATER 
#define LDA_ABSY    0xB9	//done => 2test, DOITLATER 
#define LDA_XIND    0xA1	//done
#define LDA_INDY    0xB1	//done => 2test, BUT EXTREMELY!!!, DOITLATER 

#define LDX_IMMD    0xA2	//done => 2test
#define LDX_ZRP     0xA6	//done => 2test
#define LDX_ZRPY    0xB6	//done => 2test, DOITLATER 
#define LDX_ABS     0xAE	//done => 2test
#define LDX_ABSY    0xBE	//done => 2test, DOITLATER 

#define LDY_IMMD    0xA0	//done => 2test
#define LDY_ZRP     0xA4	//done => 2test
#define LDY_ZRPX    0xB4	//done => 2test, DOITLATER 
#define LDY_ABS     0xAC	//done => 2test
#define LDY_ABSX    0xBC	//done => 2test, DOITLATER 

#define STA_ZRP     0x85 	//done => 2test
#define STA_ZRPX    0x95 	//done => 2test, DOITLATER 
#define STA_ABS     0x8D 	//done => 2test
#define STA_ABSX    0x9D 	//done => 2test, DOITLATER 
#define STA_ABSY    0x99 	//done => 2test, DOITLATER 
#define STA_XIND    0x81 	//DOITLATER 
#define STA_INDY    0x91 	//DOITLATER 
                
#define STX_ZRP     0x86 	//done => 2test
#define STX_ZRPY	0x96 	//DOITLATER
#define STX_ABS		0x8E	//done => 2test 
                
#define STY_ZRP     0x84 	//done => 2test
#define STY_ZRPX    0x94 	//DOITLATER
#define STY_ABS     0x8C 	//done => 2test
                						
//ARITHMETIC INSTRUCTIONS
#define ADC_IMMD    0x69 	//OK
#define ADC_ZRP     0x65 	//done => 2test
#define ADC_ZRPX    0x75 	//DOITLATER
#define ADC_ABS     0x6D 	//done => 2test
#define ADC_ABSX    0x7D 	//DOITLATER
#define ADC_ABSY    0x79 	//DOITLATER
#define ADC_XIND    0x61 	//DOITLATER
#define ADC_INDY    0x71 	//DOITLATER
                
#define SBC_IMMD    0xE9    //???
#define SBC_ZRP     0xE5 
#define SBC_ZRPX    0xF5 
#define SBC_ABS     0xED 
#define SBC_ABSX    0xFD 
#define SBC_ABSY    0xF9 
#define SBC_XIND    0xE1 
#define SBC_INDY    0xF1 
                
#define INC_ZRP     0xE6    //done
#define INC_ZRPX    0xF6    //done
#define INC_ABS     0xEE    //done
#define INC_ABSX    0xFE    //done
                
#define INX_IMPL    0xE8    //done
                
#define INY_IMPL    0xC8    //done
                
#define DEC_ZRP     0xC6    //done
#define DEC_ZRPX    0xD6    //done
#define DEC_ABS     0xCE    //done
#define DEC_ABSX    0xDE    //done
                
#define DEX_IMPL    0xCA    //done
                                
#define DEY_IMPL    0x88    //done
                
                
//SHIFT & ROTATE INSTRUCTIONS
#define ASL_ACCU    0x0A    //done  
#define ASL_ZRP     0x06    //done
#define ASL_ZRPX    0x16    //done
#define ASL_ABS     0x0E    //done
#define ASL_ABSX    0x1E    //done
                 
#define LSR_ACCU    0x4A    //done         
#define LSR_ZRP     0x46    //done
#define LSR_ZRPX    0x56    //done
#define LSR_ABS     0x4E    //done
#define LSR_ABSX    0x5E    //done
                
#define ROL_ACCU    0x2A    //done         
#define ROL_ZRP     0x26    //done
#define ROL_ZRPX    0x36    //done
#define ROL_ABS     0x2E    //done
#define ROL_ABSX    0x3E    //done
                
#define ROR_ACCU    0x6A    //done         
#define ROR_ZRP     0x66    //done
#define ROR_ZRPX    0x76    //done
#define ROR_ABS     0x6E    //done
#define ROR_ABSX    0x7E    //done
                
//LOGIC INSTRUCTIONS
#define AND_IMMD    0x29    //done
#define AND_ZRP     0x25    //done
#define AND_ZRPX    0x35    //done
#define AND_ABS     0x2D    //done
#define AND_ABSX    0x3D    //done
#define AND_ABSY    0x39    //done
#define AND_XIND    0x21    //done
#define AND_INDY    0x31 
                
#define ORA_IMMD    0x09    //done       
#define ORA_ZRP     0x05    //done       
#define ORA_ZRPX    0x15    //done       
#define ORA_ABS     0x0D    //done       
#define ORA_ABSX    0x1D    //done
#define ORA_ABSY    0x19    //done                 
#define ORA_XIND    0x01    //done              
#define ORA_INDY    0x11     
                                
#define EOR_IMMD    0x49    //done                       
#define EOR_ZRP     0x45    //done                       
#define EOR_ZRPX    0x55    //done                     
#define EOR_ABS     0x4D    //done                      
#define EOR_ABSX    0x5D    //done
#define EOR_ABSY    0x59    //done
#define EOR_XIND    0x41    //done
#define EOR_INDY    0x51
                
//COMPARE AND TEST BIT INSTRUCTIONS
#define CMP_IMMD    0xC9                                
#define CMP_ZRP     0xC5                                
#define CMP_ZRPX    0xD5                                
#define CMP_ABS     0xCD                                
#define CMP_ABSX    0xDD                               
#define CMP_ABSY    0xD9                               
#define CMP_XIND    0xC1                               
#define CMP_INDY    0xD1
               
#define CPX_IMMD    0xE0                                
#define CPX_ZRP     0xE4                                
#define CPX_ABS     0xEC                                
                
#define CPY_IMMD    0xC0                                
#define CPY_ZRP     0xC4                                
#define CPY_ABS     0xCC                                
                
#define BIT_ZRP     0x24                                
#define BIT_ABS     0x2C
            
//SET AND CLEAR INSTRUCTIONS
#define SEC_IMPL    0x38	//done
               
#define SED_IMPL    0xF8	//done
               
#define SEI_IMPL    0x78	//done
                
#define CLC_IMPL    0x18	//done
                
#define CLD_IMPL    0xD8	//done
                 
#define CLI_IMPL    0x58	//done
                
#define CLV_IMPL    0xB8	//done
                
//JUMP AND SUBROUTINE INSTRUCTIONS
#define JMP_ABS     0x4C
#define JMP_IND     0x6C
                
#define JSR_ABS     0x20    //done
                 
#define RTS_IMPL    0x60
                
#define RTI_IMPL    0x40
                
//BRANCH INSTRUCTIONS
#define BCC_REL     0x90    //done
                
#define BCS_REL     0xB0
                 
#define BEQ_REL     0xF0
                
#define BMI_REL     0x30
                
#define BNE_REL     0xD0    //done
                 
#define BPL_REL     0x10
                 
#define BVC_REL     0x50
                
#define BVS_REL     0x70
                
//STACK INSTRUCTIONS
#define PHA_IMPL    0x48    //done
                 
#define PLA_IMPL    0x68    //done
                 
#define PHP_IMPL    0x08    //done
                
#define PLP_IMPL    0x28    //done
                
//MISC INSTRUCTIONS
#define NOP_IMPL    0xEA	//done
                
#define BRK_IMPL    0x00


//TRANSFER INSTRUCTIONS (single byte instructions, operand addr is implied by opcode)
void tax(T6502 cpu);		//transfer A to X
void txa(void);    	//transfer X to A              
void tay(void);    	//transfer A to Y
void tya(void);    	//transfer Y to A                
void tsx(void);    	//(tansfer stack pointer to X) 
void txs(void);    	//(tansfer X to stack pointer) NOTE: Although many instructions modify the value of the Stack Pointer, TXS is the only way to set it to a specified value.             

//STORAGE INSTRUCTIONS
void lda(word operand);
void ldx(word operand);
void ldy(word operand);
void sta(address a);
void stx(address a);
void sty(address a);
                						
//ARITHMETIC INSTRUCTIONS
void adc(word operand);
void sbc(word operand);
                
void inc(address a);  
                
void inx    (void);  
                
void iny    (void);  
                
void dec(address a);  
                
void dex    (void);  
                                
void dey    (void);  
                
                
//SHIFT & ROTATE INSTRUCTIONS
void asl_accu(void); //done                 
void asl(address a); //done
                 
void lsr_accu(void); //done                
void lsr(address a); //done

void rol_accu(void); //done              
void rol(address a); //done
                
void ror_accu (void);//done               
void ror(address a); //done
                
//LOGIC INSTRUCTIONS
void and(word operand);    
                
void ora(word operand);
                                
void eor(word operand);
                
//COMPARE AND TEST BIT INSTRUCTIONS
void cmp (word operand);                                  
               
void cpx_immd    (void);                                  
void cpx_zrp     (void);                                  
void cpx_abs     (void);                                  
                
void cpy_immd    (void);                                  
void cpy_zrp     (void);                                  
void cpy_abs     (void);                                  
                
void bit_zrp     (void);                                  
void bit_abs     (void);  
            
//SET AND CLEAR INSTRUCTIONS
void sec    (void);  
               
void sed    (void);  
               
void sei	(void);  
                
void clc    (void);
                
void cld    (void);  
                
void cli    (void);  
                
void clv    (void);  
                
//JUMP AND SUBROUTINE INSTRUCTIONS
void jmp(address a);  
                
void jsr (address a);  
              
void rts (void);  
                
void rti_impl    (void);  
                
//BRANCH INSTRUCTIONS
void bcc(sword operand);    //done
                
void bcs(sword operand);    //done
                
void beq(sword operand);    //done
                
void bmi(sword operand);    //done
                
void bne(sword operand);    //done
                 
void bpl(sword operand);    //done
                 
void bvc(sword operand);    //done
                
void bvs(sword operand);    //done
                
//STACK INSTRUCTIONS
void pha(void);  //done
                 
void pla(void);  //done
                 
void php(void);  //done
                
void plp(void);  //done
                
//MISC INSTRUCTIONS
//void nop_impl    (void); //no function needed, just don't do anything on NOP_IMPL (0xEA)  
                
void brk_impl    (void);  
                
#endif