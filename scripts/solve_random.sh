#!/bin/bash
#SBATCH --job-name=pmpNL
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=unlimited
#SBATCH --array=0-1%2

# Activate the conda env if needed
#source /etc/profile.d/conda.sh # Required before using conda
#conda activate myenv

CMD=./build/large_PMP
TYPERAND="random_144_216"
DIR_DATA=./data/Random/${TYPERAND}/

D_MATRIX=${DIR_DATA}dist_matrix.txt
WEIGHTS=${DIR_DATA}cust_weights_3.txt
TIME_CPLEX=0
TIME_CLOCK=3600


###### mat
SERVICE="6x6"
p_values=(4)

CAPACITIES=${DIR_DATA}loc_capacities_${SERVICE}_rand.txt
OUTPUT=./solutions/test_random_${TYPERAND}_${SERVICE}/

METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD \
        -o $OUTPUT | tee ./console/console_${TYPERAND}_${SERVICE}_${METHOD}_p_${p}.txt")
done


METHOD="RSSV"
METHOD_RSSV_FINAL="VNS_CPMP"
methods=("TB_PMP" "TB_CPMP")
for p in "${p_values[@]}"
do
  for metsp in "${methods[@]}"
  do
    arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
          -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
          -o $OUTPUT | tee ./console/console_${TYPERAND}_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
  done
done

METHOD="RSSV"
METHOD_RSSV_FINAL="TB_CPMP"
methods=("TB_PMP" "TB_CPMP")
for p in "${p_values[@]}"
do
  for metsp in "${methods[@]}"
  do
    arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
          -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
          -o $OUTPUT | tee ./console/console_${TYPERAND}_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
  done
done


if [ -z "$arr" ]; then
    echo "No instances"
fi

for element in "${arr[@]}"; do
    echo "$element"
done

# echo "Number of instances: ${#arr[@]}"
#chmod +x ${arr[$SLURM_ARRAY_TASK_ID]}
# srun ${arr[$SLURM_ARRAY_TASK_ID]}