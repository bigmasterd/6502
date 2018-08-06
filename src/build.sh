rm 6502
rm *.o

#compile libs but do not link
gcc -c mem.c -o mem.o 
gcc -c utils.c -o utils.o
gcc -c test.c -o test.o

#compile main and link libs
gcc 6502.c -o 6502 mem.o utils.o test.o

#run if path to binary was given
if [ $# -gt 0 ]; then
  if [ -e $1 ]; then
    ./6502 $1
  else
    echo "Error: file "$1" not found"
  fi
fi
