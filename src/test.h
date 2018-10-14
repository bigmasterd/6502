
#include "types.h"


void preptest(word opcode);
void test(word opcode);
void check_reg(word exp_reg_val, word act_reg_value, char* reg_name, char* opcode_name);
void check_mem(address addr, word exp_mem_val, char* opcode_name);