#!/bin/sh
set -e
echo "Compiling..."
options='-c -g -Wall -fPIC -lto -Ofast -std=c++17 -DNDEBUG'
# all_options='-fprofile-generate'
all_options='-fprofile-use -fprofile-correction'
g++ $options $all_options solvers.cpp -o solvers.o
g++ $options $all_options sha256.cpp -o sha256.o
echo "Linking..."
g++ -g -shared $all_options -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o sha256.o
