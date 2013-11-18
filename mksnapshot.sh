#!/bin/bash
cd jni/external/v8
rm -rf host
mkdir -p host
cd host
scons -Y.. simulator=arm snapshot=on
mv obj/release/snapshot.cc ../../../snapshot.cc
cd ../../..
echo Copied snapshot to `pwd`/snapshot.cc