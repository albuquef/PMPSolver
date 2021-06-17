#!/bin/sh

CMD=./cmake-build-debug/large_PMP
D_MATRIX=./data/paca_v4/dist_matrix.txt
WEIGHTS=./data/paca_v4/cust_weights.txt


for p in 37 60 174 335 352 478 681 1924

p=37
CAPACITIES=./data/paca_v4/capacities_mat.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt

p=60
CAPACITIES=./data/paca_v4/capacities_urg.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt

p=174
CAPACITIES=./data/paca_v4/capacities_gend.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt

p=335
CAPACITIES=./data/paca_v4/capacities_hop.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt

p=352
CAPACITIES=./data/paca_v4/capacities_lyc.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt

p=478
CAPACITIES=./data/paca_v4/capacities_col.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt

p=681
CAPACITIES=./data/paca_v4/capacities_pos.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt

p=1924
CAPACITIES=./data/paca_v4/capacities_ep.txt
$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -mode 4 -o ./solutions/PACA_v4_cap_p=${p}.txt | tee console_cap_${p}.txt