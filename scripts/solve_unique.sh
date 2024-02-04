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

MODE=2
CMD=./build/large_PMP
DIR_DATA=./data/filterData_PACA_may23/
DIST_TYPE=minutes

D_MATRIX=${DIR_DATA}dist_matrix_${DIST_TYPE}.txt
WEIGHTS=${DIR_DATA}cust_weights.txt
TIME_CPLEX=3600
TIME_CLOCK=3600


###### mat
SERVICE=mat 
p_values=(30 42)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
OUTPUT=./solutions/test_paca_${SERVICE}

METHOD="EXACT_CPMP"
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        --mode $MODE -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
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