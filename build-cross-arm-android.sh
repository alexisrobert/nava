#!/bin/sh

if [[ ! -f "$1" ]] ; then
	echo "Usage: $0 [source]"
	exit 1
fi

if [[ "${AROOT}" == "" ]] ; then
	echo "Please set AROOT environment variable to Android NDK root"
	exit 1
fi

TEMPLL=$(mktemp "/tmp/tmp.XXXX.ll")
TEMPS=$(mktemp "/tmp/tmp.XXXX.s")

OUTPUT=$(dirname "$1")/lib$(basename "$1" ".nv").so

LLCFLAGS="-relocation-model=pic -enable-unsafe-fp-math -O3 -time-passes -stats -march=arm"

CXXFLAGS="$(dirname "$0")/stdlib/stdlib.cpp -I$AROOT/platforms/android-8/arch-arm/usr/include -fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -march=armv5te -mtune=xscale -msoft-float -mthumb -Os -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -DANDROID -Wa,-noexecstack -nostdlib -Bdynamic -Wl,-dynamic-linker,/system/bin/linker -Wl,-gc-sections -Wl,-z,nocopyreloc $AROOT/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/lib/gcc/arm-eabi/4.4.0/libgcc.a $AROOT/platforms/android-8/arch-arm/usr/lib/libc.so $AROOT/platforms/android-8/arch-arm/usr/lib/libstdc++.so $AROOT/platforms/android-8/arch-arm/usr/lib/libm.so -Wl,-no-undefined -Wl,-z,noexecstack -Wl,-rpath-link=$AROOT/platforms/android-8/arch-arm/usr/lib/ -shared"

if [[ ! -f "$(dirname "$0")/nava" ]] ; then
	echo "nava not found."
	exit 1
fi

echo "Generating LLVM bytecode ..."
($(dirname "$0")/nava < $1) >/dev/null 2>${TEMPLL}

echo "Converting bytecode to native code ..."
llc ${LLCFLAGS} -o ${TEMPS} ${TEMPLL} && $AROOT/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/arm-eabi-gcc ${TEMPS} ${CXXFLAGS} ${LDFLAGS} -o ${OUTPUT} && echo "Build OK!"

rm -f ${TEMPS} ${TEMPLL}
