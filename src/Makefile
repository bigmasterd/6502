CC=gcc

the_build: mem.o utils.o test.o 6502.o
	#link all to 6502 executalbe
	$(CC) -o 6502 -g 6502.o -g mem.o -g utils.o -g test.o
