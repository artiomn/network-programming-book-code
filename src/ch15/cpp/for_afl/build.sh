#!/bin/sh

#export CC=afl-clang-fast
#export CXX=afl-clang-fast++

cmake -DCMAKE_C_COMPILER=afl-cc -DCMAKE_CXX_COMPILER=afl-c++ -B build .
cmake -B build .
VERBOSE=1 cmake --build build --parallel  # --clean-first

