#!/bin/sh

#if (( $# < 2 )); then
	  #echo "Missing arguments! Exiting..."
#	  exit 1
#fi
writefile=$1
writestr=$2
directory="$(dirname "${writefile}")"
mkdir -p $directory
touch $writefile
if [ ! -e $writefile ]; then
	#echo "File could not be created! Exiting..."
	exit 1
fi
echo $writestr > $writefile
exit 0
