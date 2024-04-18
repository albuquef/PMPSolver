#!/bin/bash
CMD=./build/large_PMP

# Time
TIME_CPLEX=3600
TIME_CLOCK=3600
# Number of threads (not used as parameter in the code)
NUM_THREADS=8

#SUBAREA
SERVICE=randomgrid
SUBAREA=grids # arrond  canton epci commune epci
COVER_MODE=true


list_indicies=(0 1)
NUM_CUSTOMERS=(144 256)
NUM_LOCATIONS=(216 384)
# DIR_DATA=./data/Random/random_${NUM_CUSTOMERS}_${NUM_LOCATIONS}/
# D_MATRIX=${DIR_DATA}dist_matrix.txt
# WEIGHTS=${DIR_DATA}cust_weights_3.txt
TYPEDIV=("2x2" "4x4" "6x6" "8x8")


##### METHODS
# METHOD="TB_CPMP"
# METHOD="VNS_CPMP"
METHOD="EXACT_CPMP"
# METHOD="RSSV"

# METHOD_RSSV_FINAL="VNS_CPMP"
# METHOD_RSSV_FINAL="EXACT_CPMP"
metsp="TB_PMP" # Subproblem method


for indice in "${list_indicies[@]}"; do

  echo "Indice: $indice"

  num_cust=${NUM_CUSTOMERS[$indice]}
  num_loc=${NUM_LOCATIONS[$indice]}
  DIR_DATA=./data/Random/random_${num_cust}_${num_loc}/
  D_MATRIX=${DIR_DATA}dist_matrix.txt
  WEIGHTS=${DIR_DATA}cust_weights_3.txt

  for typediv in "${TYPEDIV[@]}"; do
    CAPACITIES=${DIR_DATA}loc_capacities_${typediv}_ratio.txt
    COVERAGES=${DIR_DATA}loc_coverages_${typediv}.txt
    CONSOLE_NAME="console_${SERVICE}_${num_cust}_${num_loc}_${typediv}_{METHOD}_p_${p}.txt"
    OUTPUT=./solutions/test_random_${TYPEDIV}
    
    p=0
    if [ "$typediv" = "2x2" ]; then
      p=$((num_cust / 4))
    elif [ "$typediv" = "4x4" ]; then
      p=$((num_cust / 16))
    elif [ "$typediv" = "6x6" ]; then
      p=$((num_cust / 36))
    elif [ "$typediv" = "8x8" ]; then
      p=$((num_cust / 64))
    fi



    arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
            -cover $COVERAGES -subarea $SUBAREA -cover_mode $COVER_MODE\
            -time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS\
            -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
            -o $OUTPUT | tee ./console/$CONSOLE_NAME")

  done
done



if [ -z "$arr" ]; then
    echo "No instances"
fi

for element in "${arr[@]}"; do
    echo "$element"
done
echo "Number of instances: ${#arr[@]}"

for element in "${arr[@]}"; do
    eval $element
done

# srun ${arr[$SLURM_ARRAY_TASK_ID]}