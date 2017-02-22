#!/bin/sh
g++ -c -fPIC -O3 -std=c++17 -DNDEBUG solvers.cpp -o solvers.o
g++ -shared -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o
