#!/bin/sh 
if [[ $# != 1 ]]
then
  echo "Please input file name."
  exit
fi

if [ ! -f $1 ]; then
  echo "File not found!"
  exit
fi

cat $1 | sed -e "/^[0-9]/d" -e "/^Weight/d"

