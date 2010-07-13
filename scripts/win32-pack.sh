#!/bin/bash

source project.conf

cd ..
VERSION=`cat src/version.h | grep '^#.*define' | grep 'APP_VERSION' | head -1 | awk '{print $4}' | sed 's/"//g'`

#echo $VERSION
#exit 0

[ -z "$VERSION" ] && VERSION="$project_default_version"

target=$project-$VERSION-win32
mkdir $target
cp -a win32-build/src/$project.exe $additional_files $target
cp $MINGW32_DLL $target
cp $DLL $target

for lib in $QT_DLL
do
  cp $QT_LIB_PREFIX/$lib $target
done

cwd=`pwd`
zip -r ../$target.zip $target
