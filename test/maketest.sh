#!/bin/sh
RM=$1
N=$(( $2 - 1 ))
rm -f ${RM}.out
while [ $N -ge 0 ];
do
  ../rmutt -i $N $RM >> ${RM}.out
  echo >> ${RM}.out
  N=$(( $N - 1 ))
done