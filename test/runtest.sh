#!/bin/sh
RM=$1
N=$(( $2 - 1 ))
NAME=`head -1 $RM | sed -e 's/^\/\/ \+//'`
echo -n "${NAME}... "
rm -f ${RM}.run
while [ $N -ge 0 ];
do
  ../rmutt -i $N $RM >> ${RM}.run
  echo >> ${RM}.run
  N=$(( $N - 1 ))
done
if diff ${RM}.run ${RM}.out ; then
    echo ok
else
    echo failed
fi
rm ${RM}.run