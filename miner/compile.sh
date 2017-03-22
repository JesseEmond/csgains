#!/bin/sh
set -e
echo "Compiling..."
options='-c -g -Wall -fPIC -lto -Ofast -std=c++17 -DNDEBUG'
g++ $options solvers.cpp -o solvers.o
g++ $options sha256.cpp -o sha256.o
echo "Linking..."
g++ -g -shared -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o sha256.o
