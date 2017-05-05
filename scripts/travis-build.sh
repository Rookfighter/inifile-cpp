#!/bin/bash -e

CURRDIR=$PWD
BASEDIR=$(cd "$(dirname "$0")"; pwd)
cd "$BASEDIR/.."

mkdir build
cd build
cmake -DBUILD_TEST=true ..
make
./TestIniFileCpp
