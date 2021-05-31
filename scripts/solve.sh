#!/bin/sh

CMD=./cmake-build-debug/large_PMP
D_MATRIX=./data/toulon/dist_matrix.txt
WEIGHTS=./data/toulon/weights_labeled.txt

for p in 37 60 174 335 352 478 681 1924
do
  $CMD -p $p -dm $D_MATRIX -w $WEIGHTS -o ./solutions/PACA_v3_p=${p}.txt
done