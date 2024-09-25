#!/bin/bash
#SBATCH --job-name=pmpPACA
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=20
#SBATCH --partition=cpuonly
#SBATCH --mem=128G
#SBATCH --time=100:00:00
#SBATCH --array=0-2%3

# Activate Conda environment (uncomment if needed)
# source /etc/profile.d/conda.sh
# conda activate myenv

# ----------------------------------- Default values -----------------------------------
# Executable
CMD=${CMD:-"./build/large_PMP"}

# Data directory and files
DIR_DATA=${DIR_DATA:-"./data/PACA_jul24/"}
DIST_TYPE=${DIST_TYPE:-"minutes"}
MAX_ID_LOC_CUST=${MAX_ID_LOC_CUST:-2037}
N=${N:-2037}
D_MATRIX=${D_MATRIX:-"${DIR_DATA}dist_matrix_${DIST_TYPE}_${MAX_ID_LOC_CUST}.txt"}
WEIGHTS=${WEIGHTS:-"${DIR_DATA}cust_weights_PACA_${MAX_ID_LOC_CUST}.txt"}

# Time settings
TIME_CPLEX=${TIME_CPLEX:-3600}  # Default CPLEX time in seconds
TIME_CLOCK=${TIME_CLOCK:-3600}

# Machine configuration
SEED=${SEED:-0}
NUM_THREADS=${NUM_THREADS:-20}
ADD_TYPE_TEST=${ADD_TYPE_TEST:-"PACA"}

# Instance configuration
SERVICES=${SERVICES:-("urgenc")}
p_values_cinema=${p_values_cinema:-("50" "58" "96" "134" "173" "192" "211" "250" "288" "326" "500" "900")}

# Cover mode settings
COVER_MODE=${COVER_MODE:-1}
KMEANS_COVER_MODE=${KMEANS_COVER_MODE:-0}
GRID_COVER_MODE=${GRID_COVER_MODE:-0}
SUBAREAS=${SUBAREAS:-("EPCI")}
COVER_MODE_N2=${COVER_MODE_N2:-0}
SUBAREAS_N2=${SUBAREAS_N2:-"commune"}

# Objective settings
IsWeighted_OBJ=${IsWeighted_OBJ:-true}
VERBOSE=${VERBOSE:-true}

# Method configuration
FOR_METHODS=${FOR_METHODS:-("FORMULATION")}
METHOD_RSSV_FINAL=${METHOD_RSSV_FINAL:-"EXACT_CPMP"}
METHOD_POSTOPT=${METHOD_POSTOPT:-"EXACT_CPMP"}
BW_MULTIPLIER=${BW_MULTIPLIER:-0.5}
FIXED_THRESHOLD_DIST=${FIXED_THRESHOLD_DIST:-0}

# Report and callback options
ADD_GENERATE_REPORTS=${ADD_GENERATE_REPORTS:-false}
ADD_BREAK_CALLBACK=${ADD_BREAK_CALLBACK:-false}
MAXDIST_STRATEGY_RSSV=${MAXDIST_STRATEGY_RSSV:-"maxmax"}
CUTS_TYPE=${CUTS_TYPE:-"none"}

# Problem size settings
SUB_PROB_SIZE=${SUB_PROB_SIZE:-800}
FINAL_PROB_RSSV_SIZE=${FINAL_PROB_RSSV_SIZE:-0}
TIME_SUBP_RSSV=${TIME_SUBP_RSSV:-0}
MAX_ITE_SUBP_RSSV=${MAX_ITE_SUBP_RSSV:-0}

# ----------------------------------- Functions -----------------------------------

# Function to handle cover mode settings
setup_cover_mode() {
  local subar=$1
  COVERAGES="${DIR_DATA}loc_coverages_${subar}_reindexed.txt"
  
  if [ "$COVER_MODE_N2" = "1" ]; then
    COVERAGES_N2="${DIR_DATA}loc_coverages_${SUBAREAS_N2}_${MAX_ID_LOC_CUST}.txt"
  fi
  
  if [ "$KMEANS_COVER_MODE" = "1" ]; then
    COVERAGES="${DIR_DATA}loc_coverages_kmeans_${subar}.txt"
    subar="kmeans_${subar}"
  fi

  if [ "$GRID_COVER_MODE" = "1" ]; then
    COVERAGES="${DIR_DATA}loc_coverages_grid_${subar}.txt"
    subar="grid_${subar}"
  fi

  if [ "$subar" = "null" ]; then
    COVER_MODE=0
    KMEANS_COVER_MODE=0
    GRID_COVER_MODE=0
  else
    COVER_MODE=1
  fi
}

# ----------------------------------- Main Loop -----------------------------------
arr=()
console_names=()

for METHOD in "${FOR_METHODS[@]}"; do
  if [ "$METHOD" = "RSSV" ]; then
    ADD_GENERATE_REPORTS=true
    ADD_BREAK_CALLBACK=true
    TIME_SUBP_RSSV=180
  fi

  for serv in "${SERVICES[@]}"; do
    for subar in "${SUBAREAS[@]}"; do
      setup_cover_mode "$subar"

      CAPACITIES="${DIR_DATA}loc_capacities_cap_${serv}_${MAX_ID_LOC_CUST}.txt"

      NEW_DIR_CONSOLE="./console/$(date '+%Y-%m-%d')_console_${ADD_TYPE_TEST}"
      mkdir -p "$NEW_DIR_CONSOLE"
      NEW_DIR="./outputs/solutions/$(date '+%Y-%m-%d')_${ADD_TYPE_TEST}"
      mkdir -p "$NEW_DIR/VarsValues_cplex/" "$NEW_DIR/Results_cplex/" "$NEW_DIR/Assignments/"

      OUTPUT="${NEW_DIR}/test_paca_${serv}_${subar}"
      [ "$COVER_MODE_N2" = "1" ] && OUTPUT="${NEW_DIR}/test_paca_${serv}_${subar}_${SUBAREAS_N2}"
      
      case "$serv" in
        mat) p_values=("${p_values_mat[@]}") ;;
        urgenc) p_values=("${p_values_urgenc[@]}") ;;
        lycee) p_values=("${p_values_lycee[@]}") ;;
        poste) p_values=("${p_values_poste[@]}") ;;
        cinema) p_values=("${p_values_cinema[@]}") ;;
        terrainsGJ) p_values=("${p_values_terrainsGJ[@]}") ;;
      esac

      for p in "${p_values[@]}"; do
        CONSOLE_NAME="console_${serv}_${METHOD}_p_${p}.log"
        [ "$subar" != "null" ] && [ "$COVER_MODE" = "1" ] && CONSOLE_NAME="console_${serv}_${METHOD}_${subar}_p_${p}.log"
        console_names+=("$CONSOLE_NAME")

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


