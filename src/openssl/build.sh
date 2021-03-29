#!/bin/bash

export ROOTDIR="$(pwd)/../"
export PKG_CONFIG_PATH=${ROOTDIR}/build/lib/pkgconfig
LIBNAME="libssl"

if [  -f ${ROOTDIR}/build/lib/${LIBNAME}.a ]; then
	echo "${LIBNAME} already exist in build/lib"
    exit
fi

VERSION="1.1.0h"
if [ ! -f openssl-${VERSION}.tar.gz ]; then
    wget https://www.openssl.org/source/openssl-${VERSION}.tar.gz
fi

rm -rf openssl-${VERSION}
tar xvf openssl-${VERSION}.tar.gz

cd openssl-${VERSION}

./config \
    --prefix=${ROOTDIR}/build \
    --libdir=lib \
    no-shared

make
make install_sw
