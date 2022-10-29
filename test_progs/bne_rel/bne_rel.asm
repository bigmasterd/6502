nop
here brk ;exit the program here, endless loop otherwise, PC must be 2 at test start   
nop
nop
nop
nop
nop
bne here   ;assuming that Z is cleared, must branch to PC-7, i.e. the BRK and exit
nop
