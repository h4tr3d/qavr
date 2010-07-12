#!/bin/bash

source project.conf

mkdir ../win32-build > /dev/null 2>&1
cd ../win32-build

rm CMakeCache.txt > /dev/null 2>&1
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/win32-x-mingw32.cmake ..

echo "*** Now you can type 'cd ../win32-build && make' for build"
