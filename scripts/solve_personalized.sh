#!/bin/bash
#SBATCH --job-name=pmp
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=90:00:00 
#SBATCH --array=0-20%5

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
p_values=(26 30 34 38 42)

CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
OUTPUT=./solutions/test_paca_${SERVICE}

# METHOD="EXACT_CPMP"
# for p in "${p_values[@]}"
# do
#   arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#         -method $METHOD \
#         -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
# done


# METHOD="RSSV"
# METHOD_RSSV_FINAL="EXACT_CPMP"
# # METHOD_RSSV_SUBP=("TB_PMP" "TB_CPMP" "VNS_PMP" "VNS_CPMP")
# methods=("TB_PMP" "TB_CPMP" "EXACT_PMP")
# for p in "${p_values[@]}"
# do
#   for metsp in "${methods[@]}"
#   do
#     arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#           -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
#           -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
#   done
# done

# METHOD="RSSV"
# METHOD_RSSV_FINAL="TB_CPMP"
# # METHOD_RSSV_SUBP=("TB_PMP" "TB_CPMP" "VNS_PMP" "VNS_CPMP")
# methods=("TB_PMP" "TB_CPMP" "EXACT_PMP")
# for p in "${p_values[@]}"
# do
#   for metsp in "${methods[@]}"
#   do
#     arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#           -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
#           -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
#   done
# done

METHOD="RSSV"
METHOD_RSSV_FINAL="VNS_CPMP"
# METHOD_RSSV_SUBP=("TB_PMP" "TB_CPMP" "VNS_PMP" "VNS_CPMP")
methods=("TB_PMP" "TB_CPMP")
for p in "${p_values[@]}"
do
  for metsp in "${methods[@]}"
  do
    arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
          -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
          -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
  done
done

# ###### urgenc
# SERVICE=urgenc 
# p_values=(42 48 54 60 66)
# CAPACITIES=${DIR_DATA}loc_capacities_cap_${SERVICE}.txt
# OUTPUT=./solutions/test_paca_${SERVICE}


# METHOD="EXACT_CPMP"
# for p in "${p_values[@]}"
# do
#   arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#         -method $METHOD \
#         -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_p_${p}.txt")
# done


# METHOD="RSSV"
# METHOD_RSSV_FINAL="EXACT_CPMP"
# # METHOD_RSSV_SUBP=("TB_PMP" "TB_CPMP" "VNS_PMP" "VNS_CPMP")
# methods=("TB_PMP" "TB_CPMP" "EXACT_PMP")
# for p in "${p_values[@]}"
# do
#   for metsp in "${methods[@]}"
#   do
#     arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#           -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
#           -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
#   done
# done


METHOD="RSSV"
METHOD_RSSV_FINAL="TB_CPMP"
# METHOD_RSSV_SUBP=("TB_PMP" "TB_CPMP" "VNS_PMP" "VNS_CPMP")
methods=("TB_PMP" "TB_CPMP")
for p in "${p_values[@]}"
do
  for metsp in "${methods[@]}"
  do
    arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
          -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
          -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
  done
done


# METHOD="RSSV"
# METHOD_RSSV_FINAL="VNS_CPMP"
# # METHOD_RSSV_SUBP=("TB_PMP" "TB_CPMP" "VNS_PMP" "VNS_CPMP")
# methods=("TB_PMP" "TB_CPMP" "EXACT_PMP")
# for p in "${p_values[@]}"
# do
#   for metsp in "${methods[@]}"
#   do
#     arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $SERVICE -time_cplex $TIME_CPLEX -time $TIME_CLOCK\
#           -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp\
#           -o $OUTPUT | tee ./console/console_${SERVICE}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt")
#   done
# done



if [ -z "$arr" ]; then
    echo "No instances"
fi

for element in "${arr[@]}"; do
    echo "$element"
done

echo "Number of instances: ${#arr[@]}"
#chmod +x ${arr[$SLURM_ARRAY_TASK_ID]}
# srun ${arr[$SLURM_ARRAY_TASK_ID]}


wait
echo "finished"
