#!/bin/sh
g++ -c -fPIC -O2 -std=c++17 solvers.cpp -o solvers.o
g++ -shared -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o
