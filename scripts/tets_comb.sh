#!/bin/bash
#SBATCH --job-name=pmp
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=90:00:00 
#SBATCH --array=0-16%5

# Activate the conda env if needed
#source /etc/profile.d/conda.sh # Required before using conda
#conda activate myenv

MODE=2
CMD=./build/large_PMP
DIR_DATA=./data/toulon/
D_MATRIX=${DIR_DATA}dist_matrix.txt
WEIGHTS=${DIR_DATA}cust_weights.txt
CAPACITIES=${DIR_DATA}loc_capacities.txt
OUTPUT=./solutions/test_combinations_toulon
TIME_CPLEX=60
TIME_CLOCK=60
p=5

methods=("TB_PMP" "TB_CPMP" "VNS_PMP" "VNS_CPMP EXACT_PMP EXACT_CPMP")
for meth in "${methods[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
         -method $meth \
        --mode $MODE -o $OUTPUT | tee ./console/console_${SERVICE}_p_${p}.txt")
done


METHOD="RSSV"
METHOD_RSSV_FINAL="EXACT_CPMP"
for metsp in "${methods[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
         -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        --mode $MODE -o $OUTPUT | tee ./console/console_${SERVICE}_p_${p}.txt")
done

METHOD="RSSV"
METHOD_RSSV_FINAL="VNS_CPMP"
for metsp in "${methods[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
         -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        --mode $MODE -o $OUTPUT | tee ./console/console_${SERVICE}_p_${p}.txt")
done


METHOD="RSSV"
METHOD_RSSV_FINAL="TB_PMP"
for metsp in "${methods[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
         -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
        --mode $MODE -o $OUTPUT | tee ./console/console_${SERVICE}_p_${p}.txt")
done



if [ -z "$arr" ]; then
    echo "No instances"
fi

for element in "${arr[@]}"; do
    echo "$element && wait"
done

echo "Number of instances: ${#arr[@]}"
#chmod +x ${arr[$SLURM_ARRAY_TASK_ID]}
# srun ${arr[$SLURM_ARRAY_TASK_ID]}


wait
echo "finished"