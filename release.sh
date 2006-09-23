#!/bin/sh
# this script is used to build releases. it requires access to a
# non-public cvs repository.
VERSION=`./rmutt -v 2>&1 | sed -e 's/rmutt version //'`
DIR="rmutt-${VERSION}"
RELEASE_DIR=releases
cvs export -D now -d ${DIR} rmutt
cd ${DIR}
rm release.sh
make clean test
make clean
cd ..
mkdir -p ${RELEASE_DIR}
tar zcf ${RELEASE_DIR}/${DIR}.tar.gz ${DIR}
zip -r ${RELEASE_DIR}/${DIR}.zip ${DIR}
rm -rf ${DIR}
