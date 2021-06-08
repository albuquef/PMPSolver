#!/bin/sh

CMD=./cmake-build-debug/large_PMP
D_MATRIX=./data/paca_v3/dist_matrix.txt
WEIGHTS=./data/paca_v3/weights_labeled.txt

# 37 60
for p in 174 335 352 478 681 1924
do
  $CMD -p $p -dm $D_MATRIX -w $WEIGHTS -o ./solutions/PACA_v3_p=${p}.txt | tee console.txt
done