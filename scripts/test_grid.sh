#!/bin/bash
#SBATCH --job-name=pmpGrid
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=100:00:00 
# #SBATCH --array=0-17%5
#SBATCH --array=0-23%4

# Activate the conda env if needed
# source /etc/profile.d/conda.sh # Required before using conda
# conda activate myenv

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
COVER_MODE=0


list_indicies=(0 1 2 3 4 5)
NUM_CUSTOMERS=(576 784 1296 1600 1936 2304)
NUM_LOCATIONS=(864 1176 1944 2400 2904 3456)
# DIR_DATA=./data/Random/random_${NUM_CUSTOMERS}_${NUM_LOCATIONS}/
# D_MATRIX=${DIR_DATA}dist_matrix.txt
# WEIGHTS=${DIR_DATA}cust_weights_3.txt
# TYPEDIV=("2x2" "4x4" "6x6" "8x8")
TYPEDIV=("2x2" "4x4")


##### METHODS
# METHOD="TB_CPMP"
# METHOD="VNS_CPMP"
# METHOD="EXACT_CPMP"
METHOD="RSSV"

# METHOD_RSSV_FINAL="VNS_CPMP"
# METHOD_RSSV_FINAL="EXACT_CPMP"
metsp="TB_PMP" # Subproblem method

for METHOD_RSSV_FINAL in "VNS_CPMP" "TB_CPMP"; do
  for indice in "${list_indicies[@]}"; do

    num_cust=${NUM_CUSTOMERS[$indice]}
    num_loc=${NUM_LOCATIONS[$indice]}
    DIR_DATA=./data/Random/random_${num_cust}_${num_loc}/
    D_MATRIX=${DIR_DATA}dist_matrix.txt
    WEIGHTS=${DIR_DATA}cust_weights_3.txt

    for typediv in "${TYPEDIV[@]}"; do

      if [ "$typediv" = "2x2" ]; then
        p=$((num_cust / 4))
      elif [ "$typediv" = "4x4" ]; then
        p=$((num_cust / 16))
      elif [ "$typediv" = "6x6" ]; then
        p=$((num_cust / 36))
      elif [ "$typediv" = "8x8" ]; then
        p=$((num_cust / 64))
      fi

      CAPACITIES=${DIR_DATA}loc_capacities_${typediv}_ratio.txt
      COVERAGES=${DIR_DATA}loc_coverages_${typediv}.txt
      CONSOLE_NAME="console_${SERVICE}_${num_cust}_${num_loc}_${typediv}_${METHOD}_p_${p}.txt"
      OUTPUT=./outputs/solutions/test_${SERVICE}_${num_cust}_${num_loc}_${typediv}_${METHOD}_p_${p}
      
      # echo "typediv: $typediv"
      # echo "num_cust: $num_cust"
      # echo "p: $p"



      arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
              -cover $COVERAGES -subarea $SUBAREA -cover_mode $COVER_MODE\
              -time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS\
              -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
              -o $OUTPUT | tee ./console/$CONSOLE_NAME")

    done
  done
done



if [ -z "$arr" ]; then
    echo "No instances"
fi

# for element in "${arr[@]}"; do
#     echo "$element"
# done
echo "Number of instances: ${#arr[@]}"

# for element in "${arr[@]}"; do
#     eval $element
# done

# srun ${arr[$SLURM_ARRAY_TASK_ID]}