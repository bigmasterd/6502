; 
; An 8-bit count down loop
;
start LDX #$FF      ; load X with $FF = 255 = = 0
loop DEX            ; X = X - 1
BNE loop            ; if X not zero then goto loop
RTS                 ; return
