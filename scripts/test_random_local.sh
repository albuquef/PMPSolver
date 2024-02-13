CMD=./build/large_PMP
NUM_CUSTOMERS=144
NUM_LOCATIONS=216
DIR_DATA=./data/Random/random_${NUM_CUSTOMERS}_${NUM_LOCATIONS}/


D_MATRIX=${DIR_DATA}dist_matrix.txt
WEIGHTS=${DIR_DATA}cust_weights_3.txt
TYPEDIV="2x2"
CAPACITIES=${DIR_DATA}loc_capacities_${TYPEDIV}_ratio.txt
TIME_CPLEX=3600
TIME_CLOCK=3600

###### mat
SERVICE=mat # lycee, mat, poste, urgenc
SUBAREA=grid # arrond  canton epci commune epci
p_values=(36)

# CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
COVERAGES=${DIR_DATA}loc_coverages_${TYPEDIV}.txt
OUTPUT=./solutions/test_random_${TYPEDIV}

METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        -cover $COVERAGES -subarea $SUBAREA\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        -o $OUTPUT | tee ./console/console_${SUBAREA}_${TYPEDIV}_${METHOD}_p_${p}.txt")
done

# METHOD="EXACT_CPMP"
# METHOD="RSSV"
# METHOD_RSSV_FINAL="VNS_CPMP"
# metsp="TB_PMP"
# for p in "${p_values[@]}"
# do
#   arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
#         -cover $COVERAGES -subarea $SUBAREA\
#         -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#         -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
#         -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
# done


for element in "${arr[@]}"; do
    echo "$element"
done

for element in "${arr[@]}"; do
    $element
done