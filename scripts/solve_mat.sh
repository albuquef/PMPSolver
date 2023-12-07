#!/bin/sh

CMD=./build/large_PMP
D_MATRIX=./data/filterData_PACA_may23/dist_matrix_minutes.txt
WEIGHTS=./data/filterData_PACA_may23/cust_weights.txt
CAPACITIES=./data/filterData_PACA_may23/loc_capacities_cap_mat.txt
OUTPUT=./solutions/test_paca_mat

p_values=(26 30 34 38 42)
for p in "${p_values[@]}"
do
  $CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES --mode 2 -o $OUTPUT | tee ./console/console_mat_p_${p}.txt
done