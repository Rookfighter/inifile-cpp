#!/bin/bash -e

CURRDIR=$PWD
BASEDIR=$(cd "$(dirname "$0")"; pwd)
cd "$BASEDIR/.."

export INSTALL_PREFIX="$PWD/dep"

echo "Getting dependencies"
mkdir -p $INSTALL_PREFIX
cd $INSTALL_PREFIX

. $BASEDIR/get-catch.sh
