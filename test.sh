#!/bin/bash

clear
rm *.so *.bin 2>/dev/null

gcc -O3 -Wall -Wextra handy_string.c test.c -o test.bin


./test.bin



