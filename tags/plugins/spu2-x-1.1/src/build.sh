#!/bin/sh

curdir=`pwd`

echo -----------------
echo Building SPU2-X
echo -----------------

if test "${SPU2XOPTIONS+set}" != set ; then
export SPU2XOPTIONS=""
fi

echo SPU2-X is Windows only. Aborting.

if [ 1 == 0 ]
then
if [ $# -gt 0 ] && [ $1 = "all" ]
then

aclocal
automake -a
autoconf
./configure ${SPU2XOPTIONS} --prefix=${PCSX2PLUGINS}
make clean
make install

else
make $@
fi

if [ $? -ne 0 ]
then
exit 1
fi
#cp libSPU2X*.so* ${PCSX2PLUGINS}
fi