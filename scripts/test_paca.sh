#!/bin/bash
#SBATCH --job-name=pmpPACA
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=100:00:00 
# #SBATCH --array=0-17%5
#SBATCH --array=0-224%7

# Activate the conda env if needed
# source /etc/profile.d/conda.sh # Required before using conda
# conda activate myenv

# Executable
CMD=./build/large_PMP
# Data
DIR_DATA=./data/filterData_PACA_may23/
DIST_TYPE=minutes
D_MATRIX="${DIR_DATA}dist_matrix_${DIST_TYPE}.txt"
WEIGHTS="${DIR_DATA}cust_weights.txt"
# Time
TIME_CPLEX=3600
TIME_CLOCK=3600
# Number of threads (not used as parameter in the code)
NUM_THREADS=8

##### Methods
# METHOD="TB_PMP"
# METHOD="VNS_CPMP"
METHOD="EXACT_CPMP"
# METHOD="RSSV"

# METHOD_RSSV_FINAL="VNS_CPMP"
# METHOD_RSSV_FINAL="EXACT_CPMP"
metsp="TB_PMP" # Subproblem method

# SERVICES
SERVICES=("mat" "urgenc" "lycee" "poste")
# SERVICES=("mat" "lycee" "poste")
# SERVICES=("mat" "urgenc")
# SERVICES=("mat")

# NOT COVERAGES
# COVER_MODE=0
# SUBAREAS=("null")

# p_values_mat=(26)
p_values_mat=(26 30 33 37 41 44 48)
p_values_urgenc=(42 48 54 60 66 72 78)
p_values_lycee=(246 281 316 352 387 422 457)
p_values_poste=(476 544 612 681 749 817 885)


# COVERAGES
COVER_MODE=1
# kmeans_cover=1
kmeans_cover=0
SUBAREAS=("arrond" "EPCI" "canton" "commune")
# SUBAREAS="EPCI"
# p_values_mat_arrond=(26)

COVER_MODE_N2=0
SUBAREAS_N2="canton"

##### Values of p
# p_values_mat=(26 30 34 38 42 46 50 51 54 58 62)
# p_values_urgenc=(42 48 54 60 66 72 78)
# p_values_mat_arrond=(26 30 33 37 41 44 48)
# p_values_mat_epci=(33 37 41 44 48 51 54)
# p_values_mat_epci=(26 30 33 37 41 44 48)
# p_values_urgenc_arrond=(42 48 54 60 66 72 78)
# p_values_urgenc_epci=(42 48 54 60 66 72 78)



# METHOD="RSSV"
# for METHOD_RSSV_FINAL in "EXACT_CPMP" "VNS_CPMP" "TB_CPMP"; do
for METHOD in "EXACT_CPMP" "RSSV"; do
  METHOD_RSSV_FINAL="EXACT_CPMP"
  for serv in "${SERVICES[@]}"; do
    for subar in "${SUBAREAS[@]}"; do
      CAPACITIES="${DIR_DATA}loc_capacities_cap_${serv}.txt"
      COVERAGES="${DIR_DATA}loc_coverages_${subar}.txt"
      # if kmeans_cover= 1, then the coverages are the kmeans coverages
      if [ $kmeans_cover = 1 ]; then
        COVERAGES="${DIR_DATA}loc_coverages_kmeans_${subar}.txt"
        subar="kmeans_${subar}"
      fi
      COVERAGES_N2="${DIR_DATA}loc_coverages_${SUBAREAS_N2}.txt"
      OUTPUT="./outputs/solutions/test_paca_${serv}_${subar}"

      # if [ $kmeans_cover = 1 ]; then
      #   OUTPUT="./solutions/test_paca_${serv}_kmeans_${subar}"
      # fi


      if [ "$SUBAREAS_N2" != "null" ]; then
          OUTPUT="./outputs/solutions/test_paca_${serv}_${subar}_${SUBAREAS_N2}"
      fi

      if [ "$subar" = "null" ]; then
        COVERAGES="${DIR_DATA}loc_coverages.txt"
        OUTPUT="./outputs/solutions/test_paca_${serv}"
      fi


      if [ "$serv" = "mat" ]; then
        p_values=("${p_values_mat[@]}")
      elif [ "$serv" = "urgenc" ]; then
        p_values=("${p_values_urgenc[@]}")
      elif [ "$serv" = "lycee" ]; then
        p_values=("${p_values_lycee[@]}")
      elif [ "$serv" = "poste" ]; then
        p_values=("${p_values_poste[@]}")
      fi

      # if [ "$serv" = "mat" ] && [ "$subar" = "null" ]; then
      #   p_values=("${p_values_mat[@]}")
      # elif [ "$serv" = "urgenc" ] && [ "$subar" = "null" ]; then
      #   p_values=("${p_values_urgenc[@]}")
      # elif [ "$serv" = "lycee" ] && [ "$subar" = "null" ]; then
      #   p_values=("${p_values_lycee[@]}")
      # elif [ "$serv" = "poste" ] && [ "$subar" = "null" ]; then
      #   p_values=("${p_values_poste[@]}")
      # fi

      # if [ "$serv" = "mat" ] && [ "$subar" = "arrond" ]; then
      #   p_values=("${p_values_mat_arrond[@]}")
      # elif [ "$serv" = "mat" ] && [ "$subar" = "epci" ]; then
      #   p_values=("${p_values_mat_epci[@]}")
      # elif [ "$serv" = "urgenc" ] && [ "$subar" = "arrond" ]; then
      #   p_values=("${p_values_urgenc_arrond[@]}")
      # elif [ "$serv" = "urgenc" ] && [ "$subar" = "epci" ]; then
      #   p_values=("${p_values_urgenc_epci[@]}")
      # fi


      for p in "${p_values[@]}"; do
        CONSOLE_NAME="console_${serv}_${METHOD}_p_${p}.txt"
        if [ "$subar" != "null" ] && [ "$COVER_MODE" = "1" ]; then
          CONSOLE_NAME="console_${serv}_${METHOD}_${subar}_p_${p}.txt"
        fi
        if [ "$METHOD" = "RSSV" ]; then
          CONSOLE_NAME="console_${serv}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt"
        fi
        if [ "$METHOD" = "RSSV" ] && [ "$subar" != "null" ] && [ "$COVER_MODE" = "1" ]; then
          CONSOLE_NAME="console_${serv}_${METHOD}_${METHOD_RSSV_FINAL}_${subar}_p_${p}.txt"
        fi

        arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $serv\
              -cover $COVERAGES -subarea $subar -cover_mode $COVER_MODE\
              -cover_n2 $COVERAGES_N2 -subarea_n2 ${SUBAREAS_N2} -cover_mode_n2 $COVER_MODE_N2\
              -time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS\
              -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
              -o $OUTPUT | tee ./console/$CONSOLE_NAME")
      done
    done
  done
done


if [ ${#arr[@]} -eq 0 ]; then
  echo "No instances"
fi

echo "Number of instances: ${#arr[@]}"

for element in "${arr[@]}"; do
  eval $element
done

# srun ${arr[$SLURM_ARRAY_TASK_ID]}
