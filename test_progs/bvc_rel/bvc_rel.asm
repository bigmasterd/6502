jmp $0004; initially, jump right after the BRK
here brk ;exit the program here after branch, endless loop otherwise   
nop
nop
nop
nop
nop
bvc here   ;assuming that V is cleared, must branch to BRK and exit
nop
