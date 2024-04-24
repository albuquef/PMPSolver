#!/bin/bash
# emoji=🍀
#SBATCH --job-name=pmpLIT
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=100:00:00 
# #SBATCH --array=0-17%5
#SBATCH --array=0-60%5

# Activate the conda env if needed
# source /etc/profile.d/conda.sh # Required before using conda
# conda activate myenv


# Define directory path
DIR_DATA="./data/Literature/"

# Define time settings
TIME_CPLEX=3600
TIME_CLOCK=3600
NUM_THREADS=8

# Methods
METHOD="EXACT_CPMP"
METHOD="EXACT_CPMP_BIN"
METHOD_RSSV_FINAL="EXACT_CPMP"
metsp="TB_PMP" # Subproblem method

# Executable
CMD="./build/large_PMP"
COVER_MODE=0

# Define p values for each group
for ((i = 0; i < 10; i++)); do
    p_values_group1+=(5)
done
for ((i = 0; i < 10; i++)); do
    p_values_group1+=(10)
done
p_values_group2=(10 15 25 30 30 40)
p_values_group3=(600 700 800 900 1000)
p_values_group5=(5 25 50 100 150 20 100 250 500 1000 5 15 40 70 100 20 75 150 300 500 10 50 100 200 300 10 30 75 125 200)

# Define N values for each group
for ((i = 0; i < 10; i++)); do
    N_values_group1+=(50)
done
for ((i = 0; i < 10; i++)); do
    N_values_group1+=(100)
done
N_values_group2=(100 200 300 300 402 402)
N_values_group3=(3038 3038 3038 3038 3038)
for ((i = 0; i < 5; i++)); do
    N_values_group5+=(535)
done
for ((i = 0; i < 5; i++)); do
    N_values_group5+=(4461)
done
for ((i = 0; i < 5; i++)); do
    N_values_group5+=(318)
done
for ((i = 0; i < 5; i++)); do
    N_values_group5+=(2392)
done
for ((i = 0; i < 5; i++)); do
    N_values_group5+=(1304)
done
for ((i = 0; i < 5; i++)); do
    N_values_group5+=(724)
done
# Define INSTANCE_GROUPS
# INSTANCE_GROUPS=("group1/" "group2/" "group3/" "group5/")
INSTANCE_GROUPS=("group1/")

# Iterate over each INSTANCE_GROUP
for INSTANCE_GROUP in "${INSTANCE_GROUPS[@]}"; do
    DIR_DATA_GROUP="${DIR_DATA}${INSTANCE_GROUP}"

    # Check if the directory exists
    if [ ! -d "$DIR_DATA_GROUP" ]; then
        echo "Error: Directory $DIR_DATA_GROUP does not exist."
        exit 1
    fi

    # # Get list of instance filenames
    # INSTANCE_FILENAMES=$(ls "$DIR_DATA_GROUP" | grep -vE '^loc|^cust|^dist')
    # Get list of instance filenames in alphabetical order
    INSTANCE_FILENAMES=$(ls -1 "$DIR_DATA_GROUP" | grep -vE '^loc|^cust|^dist')
    # INSTANCE_FILENAMES="p3038_1000.txt"
    INSTANCE_FILENAMES="cpmp01.txt"


    # Select p values corresponding to the group
    case "$INSTANCE_GROUP" in
        "group1/")
            p_values=("${p_values_group1[@]}")
            N_values=("${N_values_group1[@]}")
            ;;
        "group2/")
            p_values=("${p_values_group2[@]}")
            N_values=("${N_values_group2[@]}")
            ;;
        "group3/")
            p_values=($(printf "%s\n" "${p_values_group3[@]}" | sort))
            N_values=("${N_values_group3[@]}")
            ;;
        "group5/")
            # p_values=($(printf "%s\n" "${p_values_group5[@]}" | sort))
            p_values=("${p_values_group5[@]}")
            N_values=("${N_values_group5[@]}")
            ;;
        *)
            echo "Error: Unknown INSTANCE_GROUP $INSTANCE_GROUP."
            exit 1
            ;;
    esac

    # Iterate over files and corresponding p values
    index=0
    for file in $INSTANCE_FILENAMES; do
        # D_MATRIX="${DIR_DATA_GROUP}dist_matrix_${file//.dat/.txt}"
        D_MATRIX="euclidian"
        WEIGHTS="${DIR_DATA_GROUP}cust_weights_${file//.dat/.txt}"
        CAPACITIES="${DIR_DATA_GROUP}loc_capacities_${file//.dat/.txt}"
        COVERAGES="${DIR_DATA_GROUP}loc_coverages_${file//.dat/.txt}"
        serv="${file%.txt}"  # Corrected variable assignment
        subar="null"
        p="${p_values[$index]}"
        N="${N_values[$index]}"
        CONSOLE_NAME="console_${serv}_${METHOD}_p_${p}.txt"
        if [ "$METHOD" = "RSSV" ]; then
            CONSOLE_NAME="console_${serv}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.txt"
        fi

        # echo "Instance: $file"
        # echo "N: $N"
        # echo "p: $p"

        OUTPUT="./solutions/test_lit_${serv}_p_${p}"

        arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $serv \
        -cover $COVERAGES -subarea $subar -cover_mode $COVER_MODE -cust_max_id $N -loc_max_id $N\
        -time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS \
        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp \
        -o $OUTPUT | tee ./console/$CONSOLE_NAME")

        ((index++))  # Increment index for next iteration
    done
done


if [ -z "$arr" ]; then
    echo "No instances"
fi

# for element in "${arr[@]}"; do
#     echo "$element"
# done
# echo "Number of instances: ${#arr[@]}"

for element in "${arr[@]}"; do
    eval $element
done

# srun ${arr[$SLURM_ARRAY_TASK_ID]}