#!/bin/bash
set -x
SRC_FILE=`readlink -f $1`
SCRIPT_DIR=`dirname -- "$0"`
BUILD_DIR="${SCRIPT_DIR}/../build"
SRC_DIR="${SCRIPT_DIR}/../src"
cd ${BUILD_DIR}
clang -emit-llvm -S -c ${SRC_DIR}/trt.cpp -I${SRC_DIR}/include || exit
cmake .. || exit
cmake --build . --target format  || exit
cmake --build . || exit
todalu -c ${SRC_FILE} > tmp.ll || exit
llvm-as tmp.ll || exit
llc tmp.bc  || exit
g++ -O3 -no-pie tmp.s -o a.out || exit
cd -
mv ${BUILD_DIR}/a.out . || exit
set +x
