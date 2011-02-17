#!/bin/sh

cd `dirname $0`

export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH
./mudlet


