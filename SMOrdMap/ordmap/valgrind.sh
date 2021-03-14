#!/bin/bash
cd "$(dirname "$0")"
g++ -Wall -Wextra -pedantic -g -O2 test.cpp -o test
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./test

# for SERIOUS debugging!
#--vgdb-error=0
