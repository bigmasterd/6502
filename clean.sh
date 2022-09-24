#clean up all binaries

rm src/6502
find . -name '*.o' -type f -delete
find . -name '*.o65' -type f -delete
