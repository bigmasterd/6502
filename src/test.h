
#include "types.h"

#define DEF_X   0b00000001
#define DEF_Y   0b00000010
#define DEF_A   0b00000011
#define DEF_P   0b00110000 //normal default value: (N V - B D I Z C) // - always 1, B is 1 too because NES does not use decimal mode D at all
#define DEF_SP  0b00000100


void preptest(word opcode);
void test(word opcode);
void check_reg(word exp_reg_val, word act_reg_value, char* reg_name, char* opcode_name);