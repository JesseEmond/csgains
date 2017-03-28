#!/bin/sh
set -e
cd cpp
echo "Compiling..."

options='-c -g -Wall -fPIC -lto -Ofast -std=c++17 -DNDEBUG'

# use this to generate a PGO profile file
# all_options='-fprofile-generate'
all_options='-fprofile-use -fprofile-correction' # correction because the profile gets broken multiple threads

g++ $options $all_options solvers.cpp -o solvers.o
g++ $options $all_options sha256.cpp -o sha256.o

echo "Linking..."
g++ -g -shared $all_options -Wl,-soname,libsolvers.o -o libsolvers.so solvers.o sha256.o
