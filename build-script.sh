#!/bin/sh

if [[ ! -f "$1" ]] ; then
	echo "Usage: $0 [source]"
	exit 1
fi

TEMPLL=$(mktemp "/tmp/tmp.XXXX.ll")
TEMPS=$(mktemp "/tmp/tmp.XXXX.s")
OUTPUT=$(dirname "$1")/$(basename "$1" ".nv")
LLCFLAGS="-enable-unsafe-fp-math -O3 -time-passes -stats"

STDLIB="$(dirname "$0")/CMakeFiles/nava.dir/stdlib/stdlib.cpp.o"

if [[ ! -f ${STDLIB} ]] ; then
	echo "${STDLIB} not found."
	exit 1
elif [[ ! -f "$(dirname "$0")/nava" ]] ; then
	echo "nava not found."
	exit 1
fi

echo "Generating LLVM bytecode ..."
($(dirname "$0")/nava < $1) >/dev/null 2>${TEMPLL}

echo "Converting bytecode to native code ..."
llc ${LLCFLAGS} -o ${TEMPS} ${TEMPLL} && g++ -o ${OUTPUT} ${TEMPS} ${STDLIB} && strip -s ${OUTPUT} && echo "Build OK!"

rm -f ${TEMPS} ${TEMPLL}
