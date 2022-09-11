;a = 35 + a;

clc           ;Clear carry flag in CCR.															OK
lda #23       ;Load the byte value 0x23h (= 35) into the accumulator							OK
adc $2043     ;Add the byte of data found at memory 0x2043. Also add one if carry flag set		TODO
sta $2043     ;Store at memory 0x2043															TODO
lda $2044     ;Load the next significant byte.													OK?
adc #00       ;Add zero, but this will also add the carry if set. 								TODO
sta $2044     ;Store the high byte back.														TODO


;   Note that 6502 uses little-endian, so adc $2043 is  8a 43 20


;As von Neumann fetch/execute cycle :
;  Initialize PC to start of program (500)

;  Use Program Counter (PC) to find and retrieve instruction op-code 
;    from memory.
;    (clc - clear carry)

;  Increment Program Counter to next memory unit.

;  Decode instruction op-code (op-code indicates no operands required)

;  Execute instruction by clearing Carry bit in CCR(condition code register).


;  Use Program Counter (PC) to find and retrieve (next) instruction op-code 
;    from memory.
;    (lda # - load accumulator immediate) 

;  Increment Program Counter to next memory unit.
 
;  Decode instruction op-code  (op-code indicates it uses 1 byte operand)

;  Use Program Counter to find and fetch operand (23) from memory.

;  Increment Program counter to next memory unit.

;  Execute instruction by copying the value found into CPU's accumulator.
;    Set any appropriate status flags. Load affects zero and negative


;  Use Program Counter (PC) to find and retrieve instruction op-code 
;    from memory.  
;    (adc $ - add to accumulator with carry from memory location) 

;  Increment Program counter to next memory unit.

;  Decode instruction  (op-code indicates it uses a 2 byte operand that
;    indicates a memory location to read from)

;  Use Program Counter to find and fetch 1st byte of operand (43) from memory.
;    Store in the low byte of a 2-byte temp. register.

;  Increment Program counter to next memory unit.

;  Use Program Counter to find and fetch 2nd byte of operand (20) from memory.
;    Store in the high byte of a 2-byte temp. register.

;  Increment Program counter to next memory unit.

;  Execute instruction by :
;    Using the address retrieved ($2043) to 
;    Fetch byte of data stored there.
;    Add it to the contents of the accumulator, 
;    If carry flag set, add an additional 1,
;    Finally, change appropriate status flags.
;      Affects zero, negative, overflow, carry

;    * Because this cup leaves work in accumulator, 
;      write-back has to be performed by a separate instruction.

;  Use Program Counter (PC) to find and retrieve next instruction op-code 
;    from memory.  
;    (sto - store accumulator at specified memory location) 

;  Increment Program counter to next memory unit.

;  etc. ....
   

;As fetch/execute cycle in a 6502 CPU :

;PC - program counter             IR - instruction register
;CC - condition code register     AC - accumulator
;MAR - memory address register    MBR - memory buffer [data] register
;TL/TH - dual 8 bit temporary register.

;The MBR may also be called MDR - memory data register.
;The PC may also be called IP - instruction pointer.

;The Fetch/Execute sequence

;Fetch Step
;  PC = 0500     (clc 1 cycle)
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Read opcode = 18 into data register from memory.
;    IR<-[MBR]   Move opcode to instruction register.
;    PC<-[PC]+1  Increment PC. PC = 0501  
;                  Some cpus use the alu to increment the PC which would
;                  tie up the accumulator and add several steps.
;                  Others have a separate dedicated PC adder.

;                Decode opcode - indicates that operand is needed.

;Execute Step
;    CC[C]=0     Set carry flag of CCR to zero.
;                N Z C I D V
;                - - 0 - - -

;Fetch Step
;  PC = 0501     (lda #23 instruction  2 cycles)
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Read opcode = 9a into data register from memory.
;    IR<-[MBR]   Move opcode to instruction register.
;    PC<-[PC]+1  Increment PC. PC = 0502  

;                Decode opcode - indicates that operand is needed.

;  PC = 0502  
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Fetch byte = 23 into data register.
;    PC<-[PC]+1  Increment PC. PC = 0503  

;Execute Step
;    AC<-[MBR]   Move byte into accumulator. 
;    Change CC   Set various flags as appropriate.
;                N Z C I D V Negative and zero possibly affected.
;                + + - - - -    

;Fetch Step
;  PC = 0503     (adc $2043 instruction  4 cycles)
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Read opcode = 8a into data register from memory.
;    IR<-[MBR]   Move opcode to instruction register.
;    PC<-[PC]+1  Increment PC. PC = 0504  

;                Decode opcode - next 2 bytes are address of needed data.

;  PC = 0504  
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Fetch byte = 43 into data register.
;    TL<-[MBR]   Move retrieved data into lower half of 2 byte temp register.
;    PC<-[PC]+1  Increment PC. PC = 0505  

;  PC = 0505  
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Fetch byte = 20 into data register.
;    TH<-[MBR]   Move retrieved data into upper half of 2 byte temp register.
;    PC<-[PC]+1  Increment PC. PC = 0506  

;Execute Step
;    MAR<-[TH/TL]  Move combined address to memory address register.
;    MBR<-M[MAR]   Fetch byte at memory 2043 into data register.
;    AC<-[AC]+MBR  Add data to Accumulator
;    AC<-[AC]+CC[C] Add carry if set.
;                  For 6502, programmer is responsible for setting initial value
;                  of carry register before performing math.

;    Change CC   Set various flags as appropriate after completing both adds.
;                N Z C I D V  Negative, zero, carry, and overflow affected.
;                + + + - - + 

;Fetch Step
;  PC = 0506     (sta $2043 instruction  4 cycles)
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Fetch byte = 84 into data register.
;    IR<-[MBR]   Move opcode to instruction register.
;    PC<-[PC]+1  Increment PC. PC = 0507  

;                Decode opcode - next 2 bytes are address of needed data.

;  PC = 0507  
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Fetch byte = 44 into data register.
;    TL<-[MBR]   Store in lower half of a double width temporary register.
;    PC<-[PC]+1  Increment PC. PC = 0508  

;  PC = 0508  
;    MAR<-[PC]   Place PC contents on address bus, signal read required.
;    MBR<-M[MAR] Fetch byte = 20 into data register.
;    TH<-[MBR]   Store data in upper half of temporary register
;    PC<-[PC]+1  Increment PC. PC = 0509  

;Execute Step
;    MAR<-[TH/TL] Move combined address to memory address register.
;    MBR<-[AC]    Move data from accumulator to data register.
;                 Signal a write.

;Fetch Step
;    PC = 0509   
;                Place the PC on the address bus and signal a read. ...
