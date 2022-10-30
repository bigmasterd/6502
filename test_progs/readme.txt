This is a bunch of atomic instruction tests (almost like unit tests).
That is, each instruction is tried to be tested independently.
Some instructions, like the BCC/BNE/... require involvement of other instructions.
Test set up and execution is implemented in modules "test.c" and "6502.c".
Test mode must be enabled via TEST_MODE define in module "6502.c".
