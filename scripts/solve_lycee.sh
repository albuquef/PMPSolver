#!/bin/sh

CMD=./build/large_PMP
D_MATRIX=./data/filterData_PACA_may23/dist_matrix_minutes.txt
WEIGHTS=./data/filterData_PACA_may23/cust_weights.txt
CAPACITIES=./data/filterData_PACA_may23/loc_capacities_cap_lycee.txt
OUTPUT=./solutions/test_paca_lycee

p_values=(246 281 316 352 387)
for p in "${p_values[@]}"
do
  $CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES --mode 2 -o $OUTPUT | tee ./console/console_lycee_p_${p}.txt
done

