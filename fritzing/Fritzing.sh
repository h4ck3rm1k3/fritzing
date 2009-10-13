#!/bin/sh
appname="Fritzing"
dirname=`dirname "${0}"`

if [ "$dirname" = "." ]; then
	dirname="$PWD/$dirname"
fi

LD_LIBRARY_PATH="${dirname}/lib"
export LD_LIBRARY_PATH

"$dirname/$appname" $*
