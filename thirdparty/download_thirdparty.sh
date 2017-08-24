#!/usr/bin/env bash
TP_DIR=$(cd "$(dirname "$BASH_SOURCE")"; pwd)
cd ${TP_DIR}

PREFIX=${TP_DIR}/installed

mkdir -p "${PREFIX}/include"
mkdir -p "${PREFIX}/lib"


if [ ! -f thrift.tar.gz ]; then
    wget -O thrift.tar.gz https://github.com/apache/thrift/archive/0.9.1.tar.gz
#    wget -O thrift.tar.gz http://archive.apache.org/dist/thrift/0.9.1/thrift-0.9.1.tar.gz
fi

if [ ! -d thrift ]; then
    tar -xzf thrift.tar.gz -C ${TP_DIR}
    tar -xzvf thrift.tar.gz | head -n 1 | sed -e "s/\//\t/g" | awk '{print $1}' > thrift.path
    mv `cat thrift.path` thrift
fi

cd ${TP_DIR}/thrift;

./bootstrap.sh
./configure CXXFLAGS='-fPIC' \
    --without-qt4 --without-c_glib --without-csharp --without-java --without-erlang --without-nodejs \
    --without-lua --without-python --without-perl --without-php --without-php_extension --without-ruby \
    --without-haskell --without-go --without-d --with-cpp \
    --prefix=${TP_DIR}/installed
make clean
make install

BIN=${PREFIX}/bin/thrift

if test ! -f ${BIN}; then
    echo "install failed"
    exit 1;
fi

#${BIN} --gen cpp --out ${TP_DIR}/../ ${TP_DIR}/../hbase.thrift