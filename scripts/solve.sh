#!/bin/sh

CMD=./cmake-build-debug/large_PMP
D_MATRIX=./data/paca_v4/dist_matrix.txt
WEIGHTS=./data/paca_v4/cust_weights.txt
CAPACITIES=./data/paca_v4/capacities_col.txt

for p in 37 60 174 335 352 478 681 1924
do
  $CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 3 -o ./solutions/PACA_v4_p=${p}.txt | tee console.txt
done


