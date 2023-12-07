#!/bin/sh

CMD=./build/large_PMP
D_MATRIX=./data/Random/random_144_216/dist_matrix.txt
WEIGHTS=./data/Random/random_144_216/cust_weights_3.txt
CAPACITIES=./data/Random/random_144_216/loc_capacities_6x6_rand.txt
OUTPUT=./solutions/test_random_144

# 37 60 174 335 352 478
p_values=(5 7 11)
for p in "${p_values[@]}"
do
  $CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES --mode 2 -o $OUTPUT | tee ./console/console_p_${p}.txt
done


