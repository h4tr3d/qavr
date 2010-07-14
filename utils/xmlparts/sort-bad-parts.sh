#!/bin/bash

if [ -z "$1" ]; then
  echo "Use: $0 <dir with parts ini files>"
  exit 1
fi

cd "$1"
mkdir bad
ls -1 *.ini | while read file
do
  result=`cat $file | grep 'prog_name=' | awk -F'=' '{print $2}'`
  if [ -z "$result" ]; then
    mv $file bad
  fi
done
