CC=gcc

the_build: mem.o utils.o test.o 6502.o
	#link all to 6502 executalbe
	$(CC) -o 6502 6502.o mem.o utils.o test.o
