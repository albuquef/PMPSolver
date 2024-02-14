#!/bin/bash
#SBATCH --job-name=pmp
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=90:00:00 
#SBATCH --array=0-19%5

# Activate the conda env if needed
#source /etc/profile.d/conda.sh # Required before using conda
#conda activate myenv

MODE=2
CMD=./build/large_PMP
DIR_DATA=./data/filterData_PACA_may23/
DIST_TYPE=minutes

D_MATRIX=${DIR_DATA}dist_matrix_${DIST_TYPE}.txt
WEIGHTS=${DIR_DATA}cust_weights.txt
CAPACITIES=./data/filterData_PACA_may23/loc_capacities_cap_poste.txt

SERVICE=poste 
CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
OUTPUT=./solutions/test_paca_${SERVICE}
p_values=(476 544 612 681 749)
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        --mode $MODE -o $OUTPUT | tee ./console/console_lycee_p_${p}.txt")
done

SERVICE=lycee 
CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
OUTPUT=./solutions/test_paca_${SERVICE}
p_values=(246 281 316 351 386)
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        --mode $MODE -o $OUTPUT | tee ./console/console_lycee_p_${p}.txt")
done

SERVICE=urgenc 
CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
OUTPUT=./solutions/test_paca_${SERVICE}
p_values=(42 48 54 60 66)
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        --mode $MODE -o $OUTPUT | tee ./console/console_lycee_p_${p}.txt")
done

SERVICE=mat 
CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
OUTPUT=./solutions/test_paca_${SERVICE}
p_values=(26 30 34 38 42)
for p in "${p_values[@]}"
do
  arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE\
        --mode $MODE -o $OUTPUT | tee ./console/console_lycee_p_${p}.txt")
done

if [ -z "$arr" ]; then
    echo "No instances"
fi

echo "Number of instances: ${#arr[@]}"

#chmod +x ${arr[$SLURM_ARRAY_TASK_ID]}
srun ${arr[$SLURM_ARRAY_TASK_ID]}

wait
echo "finished"


