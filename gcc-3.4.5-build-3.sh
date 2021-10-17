#!/bin/sh


# Important: Before running this script, ensure that you have a mechanism in
# place for dealing with Cygwin's ln and soft links (such as the included ln
# script somewhere in PATH before /usr/bin/ln).


export PATH=/mingw32/bin:/mingw32/lib/gcc/i686-w64-mingw32/10.3.0:$PATH
export GCC_EXEC_PREFIX=/mingw32/lib/gcc/

cd "C:\Users\stahta01\devel\gcc-git\build-mingw-gcc"

CC=/mingw32/bin/gcc.exe \
../configure --with-gcc --with-gnu-ld --with-gnu-as \
  --host=mingw32 --target=mingw32 \
  --prefix=/mingw32/local --enable-threads \
  --disable-nls --enable-languages=c,c++,ada \
  --disable-win32-registry --disable-shared --enable-sjlj-exceptions \
  --enable-libgcj --disable-java-awt --without-x \
  --disable-libgcj-debug --enable-interpreter --enable-hash-synchronization \
  --enable-libstdcxx-debug

make CFLAGS="-I/mingw32/i686-w64-mingw32/include -O2 -fomit-frame-pointer -D__USE_MINGW_ACCESS" \
  BOOT_CFLAGS="-O2 -fomit-frame-pointer -D__USE_MINGW_ACCESS" \
  CFLAGS_FOR_TARGET="-O2 -fomit-frame-pointer -D__USE_MINGW_ACCESS" \
  BOOT_CXXFLAGS="-mthreads -fno-omit-frame-pointer -O2" \
  BOOT_LDFLAGS=-s \
  bootstrap

make -C gcc CFLAGS=-O2 LDFLAGS=-s gnatlib_and_tools
