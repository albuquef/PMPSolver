#!/bin/bash
#SBATCH --job-name=pmpPACA
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=20
#SBATCH --partition=cpuonly
#SBATCH --mem=128G
#SBATCH --time=100:00:00 
# #SBATCH --array=0-17%5
# SBATCH --array=0-69%4
#SBATCH --array=0-1%2


# Activate the conda env if needed
# source /etc/profile.d/conda.sh # Required before using conda
# conda activate myenv

# Executable
CMD=./build/large_PMP
# Data
DIR_DATA=./data/PACA_Jul24/
DIST_TYPE=minutes
MAX_ID_LOC_CUST=2037
D_MATRIX="${DIR_DATA}dist_matrix_${DIST_TYPE}_${MAX_ID_LOC_CUST}.txt"
D_MATRIX="euclidean"
WEIGHTS="${DIR_DATA}cust_weights_PACA_${MAX_ID_LOC_CUST}_Jul24.txt"
# WEIGHTS_files=("${DIR_DATA}cust_weights.txt" "${DIR_DATA}cust_weights_shuffled.txt" "${DIR_DATA}cust_weights_split.txt")
# Time
TIME_CPLEX=2400
TIME_CLOCK=3600
# Number of threads (not used as parameter in the code)


# ---------------------------------------- Machine configuration ----------------------------------------
SEED=0
NUM_THREADS=20
ADD_TYPE_TEST="PACA"

# ----------------------------------------- Instance configuration -----------------------------------------
# SERVICES=("mat" "urgenc" "lycee" "poste")
SERVICES=("cinema" "terrainsGJ")

# p_values_mat=(26 30 33 37 41 44 48)
# p_values_urgenc=(42 48 54 60 66 72 78)
# p_values_lycee=(246 281 316 352 387 422 457)
# p_values_poste=(476 544 612 681 749 817 885)
# p_values_cinema=(134 154 173 192 211 230 250)
p_values_cinema=(1000)
# p_values_terrainsGJ=(706 806 1008 1109 1210 1310)
p_values_terrainsGJ=(1500)


# COVERAGES
#  ------- COVER LEVEL 1 -------
COVER_MODE=0
KMEANS_COVER_MODE=0
GRID_COVER_MODE=0

# SUBAREAS=("commune")
# SUBAREAS=("null" "arrond" "EPCI" "canton" "commune")
# SUBAREAS=("arrond" "EPCI" "canton" "commune")
SUBAREAS="null"

#  -------  COVER LEVEL 2 -------
COVER_MODE_N2=0
SUBAREAS_N2="null"


# ------- params instance ------
N=2037
MAX_ID_LOC_CUST=2037
IsWeighted_OBJ=false


# ----------------------------------------- Methods configuration -----------------------------------------
FOR_METHODS=("RSSV" "EXACT_CPMP")
METHOD_RSSV_FINAL="EXACT_CPMP"
metsp="TB_PMP" # Subproblem method
METHOD_POSTOPT="EXACT_CPMP"


TIME_CPLEX=3600 # 1 hour
# TIME_CPLEX=2400 # 1 hour
# TIME_CLOCK=3600


SUB_PROB_SIZE=800
FIXED_THRESHOLD_DIST=0 # 0 = No fixed threshold (by default 0)
TIME_SUBP_RSSV=0 # 0 = No limit   (by default 0)
MAX_ITE_SUBP_RSSV=0 # 0 = No limit (by default 0)

BW_MULTIPLIER=0.5   # Bandwidth multiplier

ADD_THRESHOLD_DIST_SUBP_RSSV=false # Add threshold distance create by subproblems
ADD_GENERATE_REPORTS=false
ADD_BREAK_CALLBACK=false
# FIXED_THRESHOLD_DIST=7200.0 # maximum distance  between the service and the customer 2h
FIXED_THRESHOLD_DIST=0 # maximum distance  between the service and the customer 2h

FOR_METHODS=("EXACT_CPMP_BIN")
ADD_GENERATE_REPORTS=false
ADD_BREAK_CALLBACK=false
TIME_CPLEX=3600
METHOD_POSTOPT="null"

# ----------------------------------------- Main loop -----------------------------------------
arr=()
console_names=()
for METHOD in "${FOR_METHODS[@]}"; do

	if [ "$METHOD" = "RSSV" ]; then
		ADD_GENERATE_REPORTS=true
		ADD_BREAK_CALLBACK=true
		TIME_CPLEX=2400 # 1 hour
		TIME_CLOCK=3600
		METHOD_RSSV_FINAL="EXACT_CPMP"
		metsp="TB_PMP" # Subproblem method
		TIME_SUBP_RSSV=300
		METHOD_POSTOPT="EXACT_CPMP"
		N=2037
		SUB_PROB_SIZE=$(echo "scale=0; 0.4 * $N / 1" | bc)
		# SUB_PROB_SIZE=$(echo "scale=0; 1.5 * $p / 1" | bc)
		# echo "SUB_PROB_SIZE: $SUB_PROB_SIZE"
	fi

	if [ "$METHOD" = "EXACT_CPMP" ]; then
		ADD_GENERATE_REPORTS=false
		ADD_BREAK_CALLBACK=false
		TIME_CPLEX=3600
		METHOD_POSTOPT="null"
	fi


  	for serv in "${SERVICES[@]}"; do

		for subar in "${SUBAREAS[@]}"; do

			if [ "$subar" == "null" ]; then
				COVER_MODE=0
				KMEANS_COVER_MODE=0
				GRID_COVER_MODE=0
			else
				COVER_MODE=1
			fi


			CAPACITIES="${DIR_DATA}loc_capacities_cap_${serv}_${MAX_ID_LOC_CUST}_Jul24.txt"
			COVERAGES="${DIR_DATA}loc_coverages_${subar}_reindexed.txt"
			if [ "$COVER_MODE_N2" = "1" ]; then
				COVERAGES_N2="${DIR_DATA}loc_coverages_${SUBAREAS_N2}_reindexed.txt"
			fi
			# if KMEANS_COVER_MODE= 1, then the coverages are the kmeans coverages
			if [ $KMEANS_COVER_MODE = 1 ]; then
				COVERAGES="${DIR_DATA}loc_coverages_kmeans_${subar}.txt"
				subar="kmeans_${subar}"
			fi

			if [ $GRID_COVER_MODE = 1 ]; then
				COVERAGES="${DIR_DATA}loc_coverages_grid_${subar}.txt"
				subar="grid_${subar}"
			fi
			
			NEW_DIR_CONSOLE="./console/$(date '+%Y-%m-%d')_console_${ADD_TYPE_TEST}"
			mkdir -p $NEW_DIR_CONSOLE
			#create a dir with date and time
			NEW_DIR="./outputs/solutions/$(date '+%Y-%m-%d')_${ADD_TYPE_TEST}"
			mkdir -p $NEW_DIR
			mkdir -p $NEW_DIR/VarsValues_cplex/
			mkdir -p $NEW_DIR/Results_cplex/
			mkdir -p $NEW_DIR/Assignments/
			# OUTPUT="${NEW_DIR}/test_${SLURM_JOB_NAME}_${serv}"
			OUTPUT="${NEW_DIR}/test_paca_${serv}_${subar}"


			if [ "$COVER_MODE_N2" = "1"  ]; then
				OUTPUT="${NEW_DIR}/test_paca_${serv}_${subar}_${SUBAREAS_N2}"
			fi

			if [ "$COVER_MODE" != "1" ]; then
				COVERAGES="${DIR_DATA}loc_coverages.txt"
				OUTPUT="${NEW_DIR}/test_paca_${serv}"
			fi


			if [ "$serv" = "mat" ]; then
				p_values=("${p_values_mat[@]}")
			elif [ "$serv" = "urgenc" ]; then
				p_values=("${p_values_urgenc[@]}")
			elif [ "$serv" = "lycee" ]; then
				p_values=("${p_values_lycee[@]}")
			elif [ "$serv" = "poste" ]; then
				p_values=("${p_values_poste[@]}")
			elif [ "$serv" = "cinema" ]; then
				p_values=("${p_values_cinema[@]}")
			elif [ "$serv" = "terrainsGJ" ]; then
				p_values=("${p_values_terrainsGJ[@]}")
			fi

			for p in "${p_values[@]}"; do

				CONSOLE_NAME="console_${serv}_${METHOD}_p_${p}.log"
				if [ "$subar" != "null" ] && [ "$COVER_MODE" = "1" ]; then
				CONSOLE_NAME="console_${serv}_${METHOD}_${subar}_p_${p}.log"
				fi
				if [ "$METHOD" = "RSSV" ]; then
				CONSOLE_NAME="console_${serv}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.log"
				fi
				if [ "$METHOD" = "RSSV" ] && [ "$subar" != "null" ] && [ "$COVER_MODE" = "1" ]; then
				CONSOLE_NAME="console_${serv}_${METHOD}_${METHOD_RSSV_FINAL}_${subar}_p_${p}.log"
				fi

				console_names+=("$CONSOLE_NAME")

				arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $serv -bw_multiplier $BW_MULTIPLIER\
				-cover $COVERAGES -subarea $subar -cover_mode $COVER_MODE \
				-cover_n2 $COVERAGES_N2 -subarea_n2 ${SUBAREAS_N2} -cover_mode_n2 $COVER_MODE_N2\
				-time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS -IsWeighted_ObjFunc $IsWeighted_OBJ\
				-method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp -size_subproblems_rssv $SUB_PROB_SIZE\
				-add_threshold_distance_rssv $ADD_THRESHOLD_DIST_SUBP_RSSV -method_post_opt $METHOD_POSTOPT\
				-time_subprob_rssv $TIME_SUBP_RSSV -max_ite_subprob_rssv $MAX_ITE_SUBP_RSSV\
				-add_generate_reports $ADD_GENERATE_REPORTS -add_break_callback $ADD_BREAK_CALLBACK -fixed_threshold_distance $FIXED_THRESHOLD_DIST\
				-cust_max_id $MAX_ID_LOC_CUST -loc_max_id $MAX_ID_LOC_CUST\
				-o $OUTPUT --seed $SEED | tee $NEW_DIR_CONSOLE/$CONSOLE_NAME")



			done
		done
  done
done


if [ -z "$2" ]; then
    set -- "$1" "$1"
fi

if [ "$2" == "cmds" ]; then
    for element in "${arr[@]}"; do
        echo "$element"
    done
elif [ "$2" == "size" ]; then
    echo "Number of instances: ${#arr[@]}"
elif [ "$2" == "run" ]; then
    for element in "${arr[@]}"; do
        eval $element
    done
elif [ "$2" == "srun" ]; then
    NEW_DIR_CONSOLE="./console/$(date '+%Y-%m-%d')_console_${ADD_TYPE_TEST}"
    mkdir -p $NEW_DIR_CONSOLE
    srun ${arr[$SLURM_ARRAY_TASK_ID]} | tee $NEW_DIR_CONSOLE/${console_names[$SLURM_ARRAY_TASK_ID]}
fi


