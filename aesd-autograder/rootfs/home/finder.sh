#!/bin/sh
#if (( $# < 2 )); then
  #echo "Not enough arguments provided. Exiting..."
#  exit 1
#fi
filesdir=$1
searchstr=$2
X=0
Y=0
if [ -d $filesdir ];
then
  X=$(grep -rc $searchstr $filesdir | wc -l)
  Y=$(grep -rn $searchstr $filesdir | wc -l)
  echo "The number of files are $X and the number of matching lines are $Y"
  exit 0
else
  #echo "$variable1 is not a directory! Exiting..."
  exit 1
fi
