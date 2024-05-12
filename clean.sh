#clean up all binaries

rm build -rf
find . -name '*.o' -type f -delete
find . -name '*.o65' -type f -delete
