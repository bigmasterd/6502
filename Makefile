

CC = gcc
CFLAGS = -g -Wall
SRCDIR = src
TESTDIR = test
BUILDDIR = build

default: $(BUILDDIR)/6502

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/6502: $(BUILDDIR)/6502.o $(BUILDDIR)/mem.o $(BUILDDIR)/utils.o $(BUILDDIR)/loader.o $(BUILDDIR)/main.o
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test: $(BUILDDIR)/6502.o $(BUILDDIR)/mem.o $(BUILDDIR)/utils.o $(BUILDDIR)/loader.o $(TESTDIR)/main.c
	$(CC) $(CFLAGS) $^ -o $@

test: $(BUILDDIR)/test

clean:
	-rm $(BUILDDIR)/*.o
	-rm $(BUILDDIR)/6502
	-rm $(BUILDDIR)/test

