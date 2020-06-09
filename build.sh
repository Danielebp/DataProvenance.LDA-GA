#!/bin/bash

# remove any existing objs
make clean

# build WarpLDA requirements
cd ./src/LDA_Libraries/WarpLDA
./get_gflags.sh
./build.sh
cd ./release
make
cd ../../../../

# build BleiLDA requirements
make myutils.o
cd ./src/LDA_Libraries/BleiLDA
make
cd ../../../

# build solution
make


