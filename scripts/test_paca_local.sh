CMD=./build/large_PMP
DIR_DATA=./data/filterData_PACA_may23/
DIST_TYPE=minutes
NUM_THREADS=1

D_MATRIX=${DIR_DATA}dist_matrix_${DIST_TYPE}.txt
WEIGHTS=${DIR_DATA}cust_weights.txt
TIME_CPLEX=3600
TIME_CLOCK=3600

###### mat
SERVICE=mat # lycee, mat, poste, urgenc
SUBAREA=arrond # arrond  canton epci commune epci
COVER_MODE=1
p_values=(26)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
COVERAGES=${DIR_DATA}loc_coverages_${SUBAREA}.txt
OUTPUT=./solutions/test_paca_${SERVICE}_${SUBAREA}

# METHOD="EXACT_CPMP"
# METHOD="VNS_CPMP"
# # METHOD="TB_CPMP"
# for p in "${p_values[@]}"
# do
#   arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
#         -cover $COVERAGES -subarea $SUBAREA -cover_mode $COVER_MODE\
#         -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#         -method $METHOD\
#         -o $OUTPUT | tee ./console/console_${SUBAREA}_${TYPEDIV}_${METHOD}_p_${p}.txt")
# done


# METHOD="EXACT_CPMP"
# for p in "${p_values[@]}"
# do
#   arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
#         -coverages $COVERAGES -subarea $SUBAREA\
#         -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#         -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
#         -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
# done


METHOD="RSSV"
# METHOD_RSSV_FINAL="TB_CPMP"
METHOD_RSSV_FINAL="VNS_CPMP"
metsp="TB_PMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        -cover $COVERAGES -subarea $SUBAREA -cover_mode $COVER_MODE\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
done


for element in "${arr[@]}"; do
    echo "$element"
done

# for element in "${arr[@]}"; do
#   $element
# done