#!/bin/bash
# #emoji=🍀
#SBATCH --job-name=pmpLIT
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=cpuonly
#SBATCH --mem=64G
#SBATCH --time=100:00:00 
# #SBATCH --array=0-75%6
#SBATCH --array=0-38%8

# Activate the conda env if needed
# source /etc/profile.d/conda.sh # Required before using conda
# conda activate myenv

# Executable
CMD="./build/large_PMP"

# Define directory path
DIR_DATA="./data/Literature/"

# ---------------------------------------- Machine configuration ----------------------------------------
NUM_THREADS=8


# ----------------------------------------- Methods configuration -----------------------------------------
# Methods
FOR_METHODS=("EXACT_CPMP" "RSSV")
# FOR_METHODS=("EXACT_CPMP")
# FOR_METHODS=("RSSV")
METHOD_RSSV_FINAL="EXACT_CPMP_BIN"
# METHOD_RSSV_FINAL="EXACT_CPMP"
metsp="TB_PMP" # Subproblem method

# Cover mode:
# COVER_MODE=1
# subar="kmeans"
COVER_MODE=0

# Weighted sum of distances
IsWeighted_OBJ=false

# Define time settings
TIME_CPLEX=3600
TIME_CLOCK=3600


# ----------------------------------------- Instance configuration -----------------------------------------

# Define p values for each group
for ((i = 0; i < 10; i++)); do
    p_values_group1+=(5)
done
for ((i = 0; i < 10; i++)); do
    p_values_group1+=(10)
done
p_values_group2=(10 15 25 30 30 40)
p_values_group3=(600 700 800 900 1000)
p_values_group4=(74 74 148 148)
# p_values_group3=(600)
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
N_values_group4=(737 737 737 737)
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

# N_values_GB21=()
# p_values_GB21=()
for ((i = 0; i < 3; i++)); do
    N_values_GB21+=(52057)
done
p_values_GB21+=(1000)
p_values_GB21+=(100)
p_values_GB21+=(2000)
for ((i = 0; i < 3; i++)); do
    N_values_GB21+=(498378)
done
p_values_GB21+=(1000)
p_values_GB21+=(100)
p_values_GB21+=(2000)
for ((i = 0; i < 3; i++)); do
    N_values_GB21+=(104814)
done
p_values_GB21+=(1000)
p_values_GB21+=(100)
p_values_GB21+=(2000)
for ((i = 0; i < 3; i++)); do
    N_values_GB21+=(10150)
done
p_values_GB21+=(1000)
p_values_GB21+=(100)
p_values_GB21+=(2000)



# Define INSTANCE_GROUPS
# INSTANCE_GROUPS=("group1/" "group2/" "group3/" "group5/")
# INSTANCE_GROUPS=("group2/" "group3/" "group5/" "GB21/")
# INSTANCE_GROUPS=("group3/" "group4/" "group5/")
INSTANCE_GROUPS=("group4/")

mapfile -t filters < ./scripts/filter_lit.txt
#print filters
# echo "Filters: ${filters[@]}"


# ----------------------------------------- Main loop -----------------------------------------


for METHOD in "${FOR_METHODS[@]}"; do
    # Iterate over each INSTANCE_GROUP
    # METHOD_RSSV_FINAL="EXACT_CPMP_BIN"

    # echo "Method RSSV FINAL: $METHOD_RSSV_FINAL"
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
        INSTANCE_FILENAMES=$(ls -1 "$DIR_DATA_GROUP" | grep -vE '^loc|^cust|^dist|\.grd$')
        # INSTANCE_FILENAMES="p3038_600.txt"
        # INSTANCE_FILENAMES="cpmp01.txt"


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
            "group4/")
                p_values=($(printf "%s\n" "${p_values_group4[@]}" | sort))
                N_values=("${N_values_group4[@]}")
                ;;
            "group5/")
                # p_values=($(printf "%s\n" "${p_values_group5[@]}" | sort))
                p_values=("${p_values_group5[@]}")
                N_values=("${N_values_group5[@]}")
                ;;
            "GB21/")
                # p_values=($(printf "%s\n" "${p_values_GB21[@]}" | sort))
                p_values=("${p_values_GB21[@]}")
                N_values=("${N_values_GB21[@]}")
                ;;
            *)
                echo "Error: Unknown INSTANCE_GROUP $INSTANCE_GROUP."
                exit 1
                ;;
        esac


        # print filenames line by line
        # echo "Instance filenames:"
        # echo "$INSTANCE_FILENAMES"


        # Iterate over files and corresponding p values
        index=0
        for file in $INSTANCE_FILENAMES; do

                    # D_MATRIX="${DIR_DATA_GROUP}dist_matrix_${file//.dat/.txt}"
                    D_MATRIX="euclidian"

                    # Check if the file contains the word "spain": this case is not euclidian distances
                    if [[ "$file" == *"spain"* ]]; then
                        # Assign a specific name for files containing the word "spain"
                        D_MATRIX="./data/Literature/group4/dist_matrix_spain.txt"
                    fi
                    echo "D_MATRIX: $D_MATRIX"

                    file=${file//.dat/.txt}
                    WEIGHTS="${DIR_DATA_GROUP}cust_weights_${file//.dat/.txt}"
                    CAPACITIES="${DIR_DATA_GROUP}loc_capacities_${file//.dat/.txt}"
                    filename_without_extension="${file%.dat}"
                    filename_without_extension="${file%.txt}"
                    COVERAGES="${DIR_DATA_GROUP}loc_coverages_kmeans_${filename_without_extension}.txt"
                    serv="${file%.txt}"  # Corrected variable assignment
                    # subar="null"
                    p="${p_values[$index]}"
                    N="${N_values[$index]}"
                    export CONSOLE_NAME="console_${serv}_${METHOD}_p_${p}.log"
                    if [ "$METHOD" = "RSSV" ]; then
                        export CONSOLE_NAME="console_${serv}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.log"
                    fi

                    # SUB PRO SIZE IS N/2 
                    SUB_PROB_SIZE=$((N / 2)) 
                    if [ "$N" -gt 1000 ]; then
                        SUB_PROB_SIZE=$((N / 3))
                    fi
                    
                    #create a dir with date and time
                    NEW_DIR="./outputs/solutions/$(date '+%Y-%m-%d')_LIT"
                    mkdir -p $NEW_DIR
                    mkdir -p $NEW_DIR/VarsValues_cplex/
                    mkdir -p $NEW_DIR/Results_cplex/
                    mkdir -p $NEW_DIR/Assignments/
                    # OUTPUT="${NEW_DIR}/test_${SLURM_JOB_NAME}_${serv}"
                    OUTPUT="${NEW_DIR}/test_lit_${serv}"

                    # Flag to check if the file matches any filter
                    match_filter=false
                    # Loop over filters and check if the file matches
                    for filter in "${filters[@]}"; do
                        # Check if the filter is non-empty before processing
                        if [[ -n $filter ]]; then
                            if [[ $file == *$filter* ]]; then
                                # echo "Filter: $filter"
                                match_filter=true
                                break  # No need to continue checking if matched
                            fi
                        fi
                    done

                    # If the file doesn't match any filter, add it to arr
                    if $match_filter; then
                    # for filter in "${filters[@]}"; do
                    # # If the filename contains any filter pattern, set match to true
                    #     if [[ $file == *$filter* ]]; then
                        # echo "Instance: $file"
                        # echo "N: $N"
                        # echo "p: $p"
                        arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $serv \
                        -cover $COVERAGES -subarea $subar -cover_mode $COVER_MODE -cust_max_id $N -loc_max_id $N\
                        -time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS -IsWeighted_ObjFunc $IsWeighted_OBJ\
                        -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp -size_subproblems_rssv $SUB_PROB_SIZE\
                        -o $OUTPUT | tee ./console/$CONSOLE_NAME")
                    fi
                    # done

            ((index++))  # Increment index for next iteration
        done
    done
done



# ----------------------------------------- Run the instances -----------------------------------------

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