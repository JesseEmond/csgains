#!/bin/sh
set -e
echo "Compiling..."
g++ -c -g -Wall -fPIC -lto -Ofast -std=c++17 -DNDEBUG solvers.cpp -o solvers.o
echo "Linking..."
g++ -g -shared -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o
