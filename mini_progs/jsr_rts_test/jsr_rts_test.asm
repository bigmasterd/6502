
; simple program to test JSR and RTS opcodes, JSR increments memory at addr 0x12
; start address of the program must be 6 (the nop after rts)
nop
nop
inc $12; increment zeropage address 12
rts
nop
nop
jsr $2; jump to address 2, the inc subroutine
nop
nop
jsr $2; jump to address 2, the inc subroutine
