#!/bin/sh
g++ -c -fPIC solvers.cpp -o solvers.o
g++ -shared -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o
