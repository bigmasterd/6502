#include "types.h"


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
#define LDA_INDX    0xA1	//done => 2test, BUT EXTREMELY!!!, DOITLATER 
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
#define STA_INDX    0x81 	//DOITLATER 
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
#define ADC_INDX    0x61 	//DOITLATER
#define ADC_INDY    0x71 	//DOITLATER
                
#define SBC_IMMD    0xE9    //???
#define SBC_ZRP     0xE5 
#define SBC_ZRPX    0xF5 
#define SBC_ABS     0xED 
#define SBC_ABSX    0xFD 
#define SBC_ABSY    0xF9 
#define SBC_INDX    0xE1 
#define SBC_INDY    0xF1 
                
#define INC_ZRP     0xE6    //OK
#define INC_ZRPX    0xF6 
#define INC_ABS     0xEE 
#define INC_ABSX    0xFE 
                
#define INX_IMPL    0xE8 
                
#define INY_IMPL    0xC8 
                
#define DEC_ZRP     0xC6 
#define DEC_ZRPX    0xD6 
#define DEC_ABS     0xCE 
#define DEC_ABSX    0xDE 
                
#define DEX_IMPL    0xCA 
                                
#define DEY_IMPL    0x88 
                
                
//SHIFT & ROTATE INSTRUCTIONS
#define ASL_ACCU    0x0A    //ok           
#define ASL_ZRP     0x06    //ok
#define ASL_ZRPX    0x16    //ok
#define ASL_ABS     0x0E    //ok
#define ASL_ABSX    0x1E    //ok
                 
#define LSR_ACCU    0x4A                   
#define LSR_ZRP     0x46 
#define LSR_ZRPX    0x56 
#define LSR_ABS     0x4E 
#define LSR_ABSX    0x5E 
                
#define ROL_ACCU    0x2A                   
#define ROL_ZRP     0x26 
#define ROL_ZRPX    0x36 
#define ROL_ABS     0x2E 
#define ROL_ABSX    0x3E 
                
#define ROR_ACCU    0x6A                   
#define ROR_ZRP     0x66 
#define ROR_ZRPX    0x76 
#define ROR_ABS     0x6E 
#define ROR_ABSX    0x7E 
                
//LOGIC INSTRUCTIONS
#define AND_IMMD    0x29 
#define AND_ZRP     0x25 
#define AND_ZRPX    0x35 
#define AND_ABS     0x2D 
#define AND_ABSX    0x3D 
#define AND_ABSY    0x39 
#define AND_INDX    0x21 
#define AND_INDY    0x31 
                
#define ORA_IMMD    0x09                 
#define ORA_ZRP     0x05                 
#define ORA_ZRPX    0x15                 
#define ORA_ABS     0x0D                 
#define ORA_ABSX    0x1D                 
#define ORA_ABSY    0x19                 
#define ORA_INDX    0x01                 
#define ORA_INDY    0x11 
                                
#define EOR_IMMD    0x49                                 
#define EOR_ZRP     0x45                                 
#define EOR_ZRPX    0x55                               
#define EOR_ABS     0x4D                                
#define EOR_ABSX    0x5D                               
#define EOR_ABSY    0x59                               
#define EOR_INDX    0x41                               
#define EOR_INDY    0x51
                
//COMPARE AND TEST BIT INSTRUCTIONS
#define CMP_IMMD    0xC9                                
#define CMP_ZRP     0xC5                                
#define CMP_ZRPX    0xD5                                
#define CMP_ABS     0xCD                                
#define CMP_ABSX    0xDD                               
#define CMP_ABSY    0xD9                               
#define CMP_INDX    0xC1                               
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
                
#define JSR_ABS     0x20
                 
#define RTS_IMPL    0x60
                
#define RTI_IMPL    0x40
                
//BRANCH INSTRUCTIONS
#define BCC_REL     0x90
                
#define BCS_REL     0xB0
                 
#define BEQ_REL     0xF0
                
#define BMI_REL     0x30
                
#define BNE_REL     0xD0
                 
#define BPL_REL     0x10
                 
#define BVC_REL     0x50
                
#define BVS_REL     0x70
                
//STACK INSTRUCTIONS
#define PHA_IMPL    0x48
                 
#define PLA_IMPL    0x68
                 
#define PHP_IMPL    0x08
                
#define PLP_IMPL    0x28
                
//MISC INSTRUCTIONS
#define NOP_IMPL    0xEA	//done
                
#define BRK_IMPL    0x00


//TRANSFER INSTRUCTIONS (single byte instructions, operand addr is implied by opcode)
void tax(void);		//transfer A to X
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
                
void dec_zrp     (void);  
void dec_zrpx    (void);  
void dec_abs     (void);  
void dec_absx    (void);  
                
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
void and_immd    (void);   
void and_zrp     (void);   
void and_zrpx    (void);   
void and_abs     (void);   
void and_absx    (void);   
void and_absy    (void);   
void and_indx    (void);   
void and_indy    (void);   
                
void ora_immd    (void);                   
void ora_zrp     (void);                   
void ora_zrpx    (void);                   
void ora_abs     (void);                   
void ora_absx    (void);                   
void ora_absy    (void);                   
void ora_indx    (void);                   
void ora_indy    (void);   
                                
void eor_immd    (void);                                   
void eor_zrp     (void);                                   
void eor_zrpx    (void);                                 
void eor_abs     (void);                                  
void eor_absx    (void);                                 
void eor_absy    (void);                                 
void eor_indx    (void);                                 
void eor_indy    (void);  
                
//COMPARE AND TEST BIT INSTRUCTIONS
void cmp_immd    (void);                                  
void cmp_zrp     (void);                                  
void cmp_zrpx    (void);                                  
void cmp_abs     (void);                                  
void cmp_absx    (void);                                 
void cmp_absy    (void);                                 
void cmp_indx    (void);                                 
void cmp_indy    (void);  
               
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
void jmp_abs     (void);  
void jmp_ind     (void);  
                
void jsr (address a);  
                 
void rts_impl    (void);  
                
void rti_impl    (void);  
                
//BRANCH INSTRUCTIONS
void bcc_rel     (void);  
                
void bcs_rel     (void);  
                
void beq_rel     (void);  
                
void bmi_rel     (void);  
                
void bne_rel     (void);  
                 
void bpl_rel     (void);  
                 
void bvc_rel     (void);  
                
void bvs_rel     (void);  
                
//STACK INSTRUCTIONS
void pha_impl(void);  //done
                 
void pla_impl(void);  //done
                 
void php_impl(void);  //done
                
void plp_impl(void);  //done
                
//MISC INSTRUCTIONS
//void nop_impl    (void); //no function needed, just don't do anything on NOP_IMPL (0xEA)  
                
void brk_impl    (void);  
                
