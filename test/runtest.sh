#!/bin/sh
RM=$1
N=$(( $2 - 1 ))
rm -f ${RM}.run
while [ $N -ge 0 ];
do
  ../rmutt -i $N $RM >> ${RM}.run
  echo >> ${RM}.run
  N=$(( $N - 1 ))
done
diff ${RM}.run ${RM}.out
rm ${RM}.run