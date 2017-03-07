#!/bin/sh
set -e
echo "Compiling..."
g++ -c -fPIC -Ofast -std=c++17 -DNDEBUG solvers.cpp -o solvers.o
echo "Linking..."
g++ -shared -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o
