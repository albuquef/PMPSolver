#!/bin/sh

CMD=./cmake-build-debug/large_PMP
D_MATRIX=./data/paca_v3/dist_matrix.txt
WEIGHTS=./data/paca_v3/cust_weights.txt
CAPACITIES=./data/paca_v3/capacities_col.txt

# 37 60 174 335 352 478
for p in 681 1924
do
  $CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 3 -o ./solutions/paca_v3_p=${p}.txt | tee ./console/console_p_${p}.txt
done


