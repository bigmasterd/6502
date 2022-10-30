jmp $0004; initially, jump right after the BRK
here brk ;exit the program here after branch, endless loop otherwise   
nop
nop
nop
nop
nop
bvs here   ;assuming that V==1, must branch to BRK and exit
nop
