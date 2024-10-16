#!/bin/bash
#SBATCH --job-name=PACAincrease
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=20
#SBATCH --partition=cpuonly
#SBATCH --mem=180GB
#SBATCH --time=100:00:00 
# #SBATCH --array=0-17%5
# SBATCH --array=0-69%4
#SBATCH --array=0-20%5

# Activate the conda env if needed
# source /etc/profile.d/conda.sh # Required before using conda
# conda activate myenv

# Executable
CMD=./build/large_PMP
# Data
DIR_DATA=./data/PACA_jul24/
DIST_TYPE=minutes
MAX_ID_LOC_CUST=2037
D_MATRIX="${DIR_DATA}dist_matrix_${DIST_TYPE}_${MAX_ID_LOC_CUST}.txt"

# WEIGHTS="${DIR_DATA}cust_weights_PACA_${MAX_ID_LOC_CUST}.txt"

# WEIGHTS="${DIR_DATA}cust_weights_PACA_${MAX_ID_LOC_CUST}_shuffle.txt"

# WEIGHTS="${DIR_DATA}cust_weights_PACA_${MAX_ID_LOC_CUST}_increase.txt"

WEIGHTS="${DIR_DATA}cust_weights_PACA_${MAX_ID_LOC_CUST}_constant.txt"
echo $WEIGHTS

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
# SERVICES=("urgenc" "cinema" "terrainsGJ")
SERVICES=("cinema")

# p_values_mat=(33 37 41 44 48)
# p_values_urgenc=(42 60 78)
# p_values_lycee=(246 281 316 352 387 422 457)
# p_values_poste=(476 544 612 681 749 817 885)
p_values_cinema=(41 46 51 56 61 96 134 173 188 192 211 230 250 288 326 500 682 767 852 937 1022)
# p_values_terrainsGJ=(706 806 1008 1109 1210 1310)

# p_values_cinema=(41 46 51 56 61 96 134 173 188 192 211 230 250 288 326 500 682 767 852 937 1022)
# COVER_MODE=1
# SUBAREAS=("commune")
# ADD_TYPE_TEST="PACA_constant_form_cinema_commune_5h_180GB_1hgapless1"


# p_values_cinema=(41 46 51 56 61 96 134 173 188 192 211 230 250 288 326 500 682 767 852 937 1022)
# COVER_MODE=1
# SUBAREAS=("canton")
# ADD_TYPE_TEST="PACA_constant_form_cinema_canton_5h_180GB_1hgapless1"

# p_values_cinema=(41 46 51 56 61 96 134 173 188 192 211 230 250 288 326 500 682 767 852 937 1022)
# COVER_MODE=1
# SUBAREAS=("EPCI")
# ADD_TYPE_TEST="PACA_constant_form_cinema_EPCI_5h_180GB_1hgapless1"

# # cover arrond 8
p_values_cinema=(41 46 51 56 61 96 134 173 188 192 211 230 250 288 326 500 682 767 852 937 1022)
COVER_MODE=1
SUBAREAS="arrond"
ADD_TYPE_TEST="PACA_constant_form_cinema_arrond_5h_180GB_1hgapless1"


# # slurm-908402 nocover
# p_values_cinema=(41 46 51 56 61 96 134 173 188 192 211 230 250 288 326 500 682 767 852 937 1022)
# COVER_MODE=0
# SUBAREAS="null"
# ADD_TYPE_TEST="PACA_constant_form_cinema_5h_180GB_1hgapless1"

echo $SUBAREAS

# COVERAGES
#  ------- COVER LEVEL 1 -------
# COVER_MODE=0
KMEANS_COVER_MODE=0
GRID_COVER_MODE=0

# # SUBAREAS=("EPCI")
# # SUBAREAS=("null" "arrond" "EPCI" "canton" "commune")
# # SUBAREAS=("arrond" "EPCI" "canton" "commune")
# SUBAREAS="null"

#  -------  COVER LEVEL 2 -------
COVER_MODE_N2=0
SUBAREAS_N2="commune"


# ------- params instance ------
N=2037
MAX_ID_LOC_CUST=2037
IsWeighted_OBJ=true
VERBOSE=true

# ----------------------------------------- Methods configuration -----------------------------------------
FOR_METHODS=("RSSV" "EXACT_CPMP")
METHOD_RSSV_FINAL="EXACT_CPMP"
metsp="TB_PMP" # Subproblem method
METHOD_POSTOPT="EXACT_CPMP"
MAXDIST_STRATEGY_RSSV="maxmax"
CUTS_TYPE="PairwiseCut_closestJ"
TIME_CPLEX=3600 # 1 hour
TIME_CLOCK=3600
BW_MULTIPLIER=0.5   # Bandwidth multiplier
FIXED_THRESHOLD_DIST=0 # 0 = No fixed threshold (by default 0)
# FIXED_THRESHOLD_DIST=7200.0 # maximum distance  between the service and the customer 2h
SUB_PROB_SIZE=800
FINAL_PROB_RSSV_SIZE=0
TIME_SUBP_RSSV=0 # 0 = No limit   (by default 0)
MAX_ITE_SUBP_RSSV=0 # 0 = No limit (by default 0)
ADD_THRESHOLD_DIST_SUBP_RSSV=false # Add threshold distance create by subproblems
ADD_GENERATE_REPORTS=false
ADD_BREAK_CALLBACK=false


# FOR_METHODS=("FORMULATION_RED" "FORMULATION_RED_POST_OPT" "RSSV_PO")
FOR_METHODS=("FORMULATION")
PROB="EXACT_CPMP"
met_sub="TB_PMP"
# CUTS_TYPES=("none" "PairwiseCut_closestJ" "PairwiseCut_allJ")
CUTS_TYPES=("none")
timesp=180


# ----------------------------------------- Main loop -----------------------------------------
arr=()
console_names=()
for CUTS_TYPE in "${CUTS_TYPES[@]}"; do
	for METHOD_TEST in "${FOR_METHODS[@]}"; do

		if [ "$METHOD_TEST" = "FORMULATION" ]; then
			METHOD=$PROB
			METHOD_POSTOPT="null"
			ADD_GENERATE_REPORTS=true

			
			ADD_BREAK_CALLBACK=true ## 1h gap less than 1%
			
			
			ADD_THRESHOLD_DIST_SUBP_RSSV=false
			TIME_CPLEX=18000 # 5 hours
			TIME_CLOCK=18000
		fi

		if [ "$METHOD_TEST" = "FORMULATION_RED" ]; then
			METHOD="RSSV"
			# metsp="TB_PMP" # Subproblem method
			metsp=$met_sub
			METHOD_RSSV_FINAL=$PROB
			METHOD_POSTOPT=$PROB
			ADD_GENERATE_REPORTS=true
			ADD_BREAK_CALLBACK=false
			ADD_THRESHOLD_DIST_SUBP_RSSV=false
			MAXDIST_STRATEGY_RSSV="minmax"
			TIME_CPLEX=3600 # 1 hour
			TIME_CLOCK=3600
			TIME_SUBP_RSSV=$timesp
			SIZE_FP_RSSV=ALL
		fi

		if [ "$METHOD_TEST" = "FORMULATION_RED_POST_OPT" ]; then
			METHOD="RSSV"
			# metsp="TB_PMP" # Subproblem method
			metsp=$met_sub
			METHOD_RSSV_FINAL=$PROB
			METHOD_POSTOPT=$PROB
			ADD_GENERATE_REPORTS=true
			ADD_BREAK_CALLBACK=true
			ADD_THRESHOLD_DIST_SUBP_RSSV=true
			TIME_CPLEX=2400 # 1 hour
			TIME_CLOCK=3600
			TIME_SUBP_RSSV=$timesp
			SIZE_FP_RSSV=ALL
		fi

		if [ "$METHOD_TEST" = "RSSV_PO" ]; then
			METHOD="RSSV"
			# metsp="TB_PMP" # Subproblem method
			metsp=$met_sub # Subproblem method
			METHOD_RSSV_FINAL=$PROB
			METHOD_POSTOPT=$PROB
			ADD_GENERATE_REPORTS=true
			ADD_BREAK_CALLBACK=true
			ADD_THRESHOLD_DIST_SUBP_RSSV=false
			TIME_CPLEX=2000 # 1 hour
			TIME_CLOCK=2000
			TIME_SUBP_RSSV=$timesp
			SIZE_FP_RSSV=DEFAULT
		fi

		if [ "$METHOD_TEST" = "RSSV_ONLY" ]; then
			METHOD="RSSV"
			# metsp="TB_PMP" # Subproblem method
			metsp=$met_sub # Subproblem method
			METHOD_RSSV_FINAL=$PROB
			METHOD_POSTOPT=$PROB
			ADD_GENERATE_REPORTS=true
			ADD_BREAK_CALLBACK=false
			ADD_THRESHOLD_DIST_SUBP_RSSV=true
			TIME_CPLEX=3600 # 1 hour
			TIME_CLOCK=3600
			TIME_SUBP_RSSV=$timesp
			SIZE_FP_RSSV=DEFAULT
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


				CAPACITIES="${DIR_DATA}loc_capacities_cap_${serv}_${MAX_ID_LOC_CUST}.txt"
				# CAPACITIES="${DIR_DATA}loc_capacities_cap_${serv}_Jul24.txt"
				COVERAGES="${DIR_DATA}loc_coverages_${subar}_${MAX_ID_LOC_CUST}.txt"
				if [ "$COVER_MODE_N2" = "1" ]; then
					COVERAGES_N2="${DIR_DATA}loc_coverages_${SUBAREAS_N2}_${MAX_ID_LOC_CUST}.txt"
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



				###### SHUFFLE ######
				OUTPUT="${NEW_DIR}/test_paca_${serv}_${subar}"




				if [ "$COVER_MODE_N2" = "1"  ]; then
					OUTPUT="${NEW_DIR}/test_paca_${serv}_${subar}_${SUBAREAS_N2}"
				fi

				if [ "$COVER_MODE" != "1" ]; then
					COVERAGES="${DIR_DATA}loc_coverages.txt"
					OUTPUT="${NEW_DIR}/test_paca_${serv}"

					###### SHUFFLE ######
					OUTPUT="${NEW_DIR}/test_paca_increase_${serv}"


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

					if [ "$SIZE_FP_RSSV" = "ALL" ]; then
						FINAL_PROB_RSSV_SIZE=$N
					else
						FINAL_PROB_RSSV_SIZE=0
					fi
					
					MIN_FP_SIZE=$(echo "(0.1 * $N + 0.999)/1" | bc) # Ceiling of 0.1*N
					SUB_PROB_SIZE=$(echo "($MIN_FP_SIZE + 2 * $p + 0.999)/1" | bc) # Ceil(0.1 * N) + 2 * p
					
					if [ "$METHOD_TEST" = "RSSV_PO" ]; then
						FINAL_PROB_RSSV_SIZE=$SUB_PROB_SIZE
						# FINAL_PROB_RSSV_SIZE=1000
					fi


					arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $serv -bw_multiplier $BW_MULTIPLIER\
					-cover $COVERAGES -subarea $subar -cover_mode $COVER_MODE \
					-cover_n2 $COVERAGES_N2 -subarea_n2 ${SUBAREAS_N2} -cover_mode_n2 $COVER_MODE_N2\
					-time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS -IsWeighted_ObjFunc $IsWeighted_OBJ\
					-method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp -size_subproblems_rssv $SUB_PROB_SIZE -size_final_prob_rssv $FINAL_PROB_RSSV_SIZE\
					-add_threshold_distance_rssv $ADD_THRESHOLD_DIST_SUBP_RSSV -method_post_opt $METHOD_POSTOPT\
					-time_subprob_rssv $TIME_SUBP_RSSV -max_ite_subprob_rssv $MAX_ITE_SUBP_RSSV\
					-add_generate_reports $ADD_GENERATE_REPORTS -add_break_callback $ADD_BREAK_CALLBACK -fixed_threshold_distance $FIXED_THRESHOLD_DIST\
					-maxdist_strategy_rssv $MAXDIST_STRATEGY_RSSV -cuts_type $CUTS_TYPE\
					-cust_max_id $MAX_ID_LOC_CUST -loc_max_id $MAX_ID_LOC_CUST --verbose $VERBOSE\
					-o $OUTPUT --seed $SEED | tee $NEW_DIR_CONSOLE/$CONSOLE_NAME")



				done
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


