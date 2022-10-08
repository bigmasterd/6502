CC = gcc
CFLAGS = -g -Wall

default: 6502

6502.o: 6502.c mem.c utils.c test.c
	$(CC) $(CFLAGS) -c 6502.c -o 6502.o
	$(CC) $(CFLAGS) -c mem.c -o mem.o
	$(CC) $(CFLAGS) -c utils.c -o utils.o
	$(CC) $(CFLAGS) -c test.c -o test.o

6502: 6502.o mem.o utils.o test.o

clean:
	-rm 6502.o mem.o utils.o test.o
	-rm 6502