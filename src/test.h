#include "types.h"


void preptest(word opcode);
void test(word opcode);
void checkReg(word exp_reg_val, word act_reg_value, char* reg_name, char* opcode_name);
void checkSP(dword exp_reg_val, dword act_reg_value, char* opcode_name);
void checkMem(address addr, word exp_mem_val, char* opcode_name);
