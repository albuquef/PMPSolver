#!/bin/bash
#SBATCH --job-name=pmpLIT
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=20
#SBATCH --partition=cpuonly
#SBATCH --mem=128G
#SBATCH --time=90:00:00
#SBATCH --array=0-19%5

# Activate the conda env if needed
source /etc/profile.d/conda.sh # Required before using conda
conda activate myenv

# Executable
CMD="./build/large_PMP"

# Define directory path
DIR_DATA="./data/Literature/"

# ---------------------------------------- Machine configuration ----------------------------------------
SEED=20
NUM_THREADS=20
ADD_TYPE_TEST="LIT"

# ----------------------------------------- Methods configuration -----------------------------------------
FOR_METHODS=("RSSV")
METHOD_RSSV_FINAL="EXACT_CPMP_BIN"
# METHOD_RSSV_FINAL="TB_CPMP"
metsp="TB_PMP" # Subproblem method

METHOD_POSTOPT="EXACT_CPMP_BIN"
# FOR_METHODS=("EXACT_CPMP_BIN")

COVER_MODE=false
IsWeighted_OBJ=false
# TIME_CPLEX=3600 # 1 hour
TIME_CPLEX=2400 # 1 hour
TIME_CLOCK=3600

ADD_THRESHOLD_DIST_SUBP_RSSV=true
# TIME_SUBP_RSSV=600 # 10 minutes  
TIME_SUBP_RSSV=300   
MAX_ITE_SUBP_RSSV=0 # 0 = No limit


TIME_SUBP_RSSV=30  
TIME_CPLEX=100 # sum the time of the subproblems
TIME_CLOCK=180 # sum of all times

BW_MULTIPLIER=0.5   # Bandwidth multiplier


# ----------------------------------------- Instance configuration -----------------------------------------
INSTANCE_GROUPS=("group2/" "group3/" "group4/" "group5/")
mapfile -t filters < ./scripts/filter_lit.txt

if [ "$1" == "basic" ]; then
    filters=("SJC4a")
    # filters=("SJC1")
    INSTANCE_GROUPS=("group2/")
elif [ "$1" == "spain" ]; then
    filters=("spain737_148_1.txt" "spain737_148_2.txt" "spain737_74_1.txt" "spain737_74_2.txt")
    INSTANCE_GROUPS=("group4/")
elif [ "$1" == "basic_spain" ]; then
    filters=("spain737_148_1.txt")
    INSTANCE_GROUPS=("group4/")
elif [ "$1" == "pr2392" ]; then
    filters=("pr2392_020.txt" "pr2392_075.txt" "pr2392_150.txt" "pr2392_300.txt" "pr2392_500.txt")
    INSTANCE_GROUPS=("group5/")
elif [ "$1" == "p3038" ]; then
    filters=("p3038_600" "p3038_700" "p3038_800" "p3038_900" "p3038_1000")
    INSTANCE_GROUPS=("group3/")
elif [ "$1" == "fnl4461" ]; then
    filters=("fnl4461_0020.txt" "fnl4461_0100.txt" "fnl4461_0250.txt" "fnl4461_0500.txt" "fnl4461_1000.txt")
    INSTANCE_GROUPS=("group5/")
elif [ "$1" == "benchmark" ]; then
    # filters=("spain737_148_1.txt" "spain737_148_2.txt" "spain737_74_1.txt" "spain737_74_2.txt")
    # filters=("SJC1" "SJC2" "SJC3a" "SJC3b" "SJC4a" "SJC4b") 
    filters=("p3038_600" "p3038_700" "p3038_800" "p3038_900" "p3038_1000")
    filters+=("rl1304_010.txt" "rl1304_050.txt" "rl1304_100.txt" "rl1304_200.txt" "rl1304_300.txt") 
    filters+=("pr2392_020.txt" "pr2392_075.txt" "pr2392_150.txt" "pr2392_300.txt" "pr2392_500.txt")
    filters+=("fnl4461_0020.txt" "fnl4461_0100.txt" "fnl4461_0250.txt" "fnl4461_0500.txt" "fnl4461_1000.txt")
    # INSTANCE_GROUPS=("group2/" "group4/" "group3/" "group5/")
    INSTANCE_GROUPS=("group3/" "group5/")
fi

# Define p values for each group
p_values_group1=(5 5 5 5 5 5 5 5 5 5 10 10 10 10 10 10 10 10 10 10)
p_values_group2=(10 15 25 30 30 40)
p_values_group3=(1000 600 700 800 900)
p_values_group4=(74 74 148 148)
p_values_group5=(5 25 50 100 150 20 100 250 500 1000 5 15 40 70 100 20 75 150 300 500 10 50 100 200 300 10 30 75 125 200)

# Define N values for each group
N_values_group1=(50 50 50 50 50 50 50 50 50 50 100 100 100 100 100 100 100 100 100 100)
N_values_group2=(100 200 300 300 402 402)
N_values_group3=(3038 3038 3038 3038 3038)
N_values_group4=(737 737 737 737)
N_values_group5=(535 535 535 535 535 4461 4461 4461 4461 4461 318 318 318 318 318 2392 2392 2392 2392 2392 1304 1304 1304 1304 1304 724 724 724 724 724)

# Define GB21 values
N_values_GB21=(52057 52057 52057 498378 498378 498378 104814 104814 104814 10150 10150 10150)
p_values_GB21=(1000 100 2000 1000 100 2000 1000 100 2000 1000 100 2000)

# ----------------------------------------- Main loop -----------------------------------------
arr=()
console_names=()

for METHOD in "${FOR_METHODS[@]}"; do
    for INSTANCE_GROUP in "${INSTANCE_GROUPS[@]}"; do
        DIR_DATA_GROUP="${DIR_DATA}${INSTANCE_GROUP}"

        if [ ! -d "$DIR_DATA_GROUP" ]; then
            echo "Error: Directory $DIR_DATA_GROUP does not exist."
            exit 1
        fi

        INSTANCE_FILENAMES=$(ls -1 "$DIR_DATA_GROUP" | grep -vE '^loc|^cust|^dist|\.grd$')

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
                p_values=("${p_values_group3[@]}")
                N_values=("${N_values_group3[@]}")
                ;;
            "group4/")
                p_values=("${p_values_group4[@]}")
                N_values=("${N_values_group4[@]}")
                ;;
            "group5/")
                p_values=("${p_values_group5[@]}")
                N_values=("${N_values_group5[@]}")
                ;;
            "GB21/")
                p_values=("${p_values_GB21[@]}")
                N_values=("${N_values_GB21[@]}")
                ;;
            *)
                echo "Error: Unknown INSTANCE_GROUP $INSTANCE_GROUP."
                exit 1
                ;;
        esac

        index=0
        for file in $INSTANCE_FILENAMES; do
            D_MATRIX="euclidian"

            if [[ "$file" == *"spain"* ]]; then
                D_MATRIX="./data/Literature/group4/dist_matrix_spain.txt"
            fi

            file=${file//.dat/.txt}
            WEIGHTS="${DIR_DATA_GROUP}cust_weights_${file//.dat/.txt}"
            CAPACITIES="${DIR_DATA_GROUP}loc_capacities_${file//.dat/.txt}"
            filename_without_extension="${file%.dat}"
            filename_without_extension="${file%.txt}"
            COVERAGES="${DIR_DATA_GROUP}loc_coverages_kmeans_${filename_without_extension}.txt"
            serv="${file%.txt}"
            p="${p_values[$index]}"
            N="${N_values[$index]}"


            if [ "$N" -lt 800 ]; then
                SUB_PROB_SIZE=$(echo "0.8 * $N" | bc)
            else
                SUB_PROB_SIZE=$(echo "0.4 * $N" | bc)
            fi


            NEW_DIR_CONSOLE="./console/$(date '+%Y-%m-%d')_console_${ADD_TYPE_TEST}"
            mkdir -p $NEW_DIR_CONSOLE
            export CONSOLE_NAME="console_${serv}_${METHOD}_p_${p}.log"
            if [ "$METHOD" = "RSSV" ]; then
                export CONSOLE_NAME="console_${serv}_${METHOD}_${METHOD_RSSV_FINAL}_p_${p}.log"
            fi


            mkdir -p ./outputs/reports/
            NEW_DIR="./outputs/solutions/$(date '+%Y-%m-%d')_${ADD_TYPE_TEST}"
            mkdir -p $NEW_DIR
            mkdir -p $NEW_DIR/VarsValues_cplex/
            mkdir -p $NEW_DIR/Results_cplex/
            mkdir -p $NEW_DIR/Assignments/
            OUTPUT="${NEW_DIR}/test_lit_${serv}"

            match_filter=false
            for filter in "${filters[@]}"; do
                if [[ -n $filter ]]; then
                    if [[ $file == *$filter* ]]; then
                        match_filter=true
                        break
                    fi
                fi
            done

            if $match_filter; then

                echo "Instance: $file"
                echo "p: $p"
                echo "N: $N"

                console_names+=("$CONSOLE_NAME")
                arr+=("$CMD -p $p -dm $D_MATRIX -w $WEIGHTS -c $CAPACITIES -service $serv -bw_multiplier $BW_MULTIPLIER\
                -cover $COVERAGES -subarea null -cover_mode $COVER_MODE -cust_max_id $N -loc_max_id $N\
                -time_cplex $TIME_CPLEX -time $TIME_CLOCK -th $NUM_THREADS -IsWeighted_ObjFunc $IsWeighted_OBJ\
                -method $METHOD -method_rssv_fp $METHOD_RSSV_FINAL -method_rssv_sp $metsp -size_subproblems_rssv $SUB_PROB_SIZE\
                -add_threshold_distance_rssv $ADD_THRESHOLD_DIST_SUBP_RSSV -method_post_opt $METHOD_POSTOPT\
                -time_subprob_rssv $TIME_SUBP_RSSV -max_ite_subprob_rssv $MAX_ITE_SUBP_RSSV\
                -o $OUTPUT --seed $SEED | tee $NEW_DIR_CONSOLE/$CONSOLE_NAME")
            fi

            ((index++))
        done
    done
done

# ----------------------------------------- Run the instances -----------------------------------------
if [ -z "$arr" ]; then
    echo "No instances"
fi

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
