#!/bin/sh

curdir=`pwd`

echo ----------------
echo Building ZeroPAD
echo ----------------

if [ $# -gt 0 ] && [ $1 = "all" ]
then

#if possible
make distclean

aclocal
automake -a
autoconf
chmod +x configure
./configure --prefix=${PCSX2PLUGINS}
make distclean
make install

else
make $@
fi

if [ $? -ne 0 ]
then
exit 1
fi

#cp libZeroPAD*.so* ${PCSX2PLUGINS}
