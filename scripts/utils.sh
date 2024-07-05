#!/bin/bash

# Source utility functions
# eval source ./scripts/utils.sh

# Constants
DATE=$(date +"%y-%m-%d")
MACHINE_CLUSTER=poseidon
USER_CLUSTER=falbuquerque
CLUSTER_SERVER="${USER_CLUSTER}@${MACHINE_CLUSTER}"
DIR_DEST_CLUSTER="test_exact_big_time_limit/"

# BASE_PATH="~/Documents/Projects/GeoAvigon/pmp_code/large-PMP"
# BASE_PATH="/home/falbuquerque/Documents/projects/Project_PMP/large-PMP/"

# SAVE_DIR="~/Documents/projects/Project_PMP/saves/SaveCluster/${DATE}_save_cluster"
# SAVE_DIR="./Documents/Projects/GeoAvigon/SaveCluster/${DATE}_save_cluster"


# Reload utility functions
reload_utils() {
    source ./scripts/utils.sh
    echo "Reloaded utils.sh"
}

# Send code to the cluster
send_code_to_cluster() {

    # call the reload function
    # reload_utils
    BASE_PATH="/home/falbuquerque/Documents/projects/Project_PMP/large-PMP/"

    if [ -z "$1" ]; then
        path="/"
    else
        path=$1
    fi
    # path="/"
    # path="/src/"
    # path="/scripts/"


    PATH_ORIGIN=${BASE_PATH}/${path}/
    echo $PATH_ORIGIN
    # PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/Benchmark_Lit${path}
    PATH_DEST=" ${CLUSTER_SERVER}:/users/falbuquerque/Projects/Project_PMP/${DIR_DEST_CLUSTER}/${path}"

    # Copy the code to the cluster
    eval "rsync -av --exclude='.git' $PATH_ORIGIN $PATH_DEST"
    # copy the code to the cluster using scp
    # eval "scp -r $PATH_ORIGIN $PATH_DEST"

    # rm -r build/ 
    # mkdir build && cd build/ && cmake "rm -r build/"

}

save_outputs_cluster() {

    # call the reload function
    reload_utils

    DIR_NAME="test_exact_big_time_limit/"
    PATH_CLUSTER=${CLUSTER_SERVER}:/users/falbuquerque/Projects/Project_PMP/${DIR_NAME}

    # PATH_LOCAL=~/Documents/projects/Project_PMP/SaveCluster/${DATE}_save_cluster/${DIR_NAME}
    PATH_LOCAL=home/felipe//Documents/Projects/GeoAvigon/save_cluster/${DATE}_save_cluster/${DIR_NAME}
    mkdir -p $PATH_LOCAL

    echo $PATH_LOCAL

    # Save the outputs to the local machine
    # eval "scp -r $PATH_CLUSTER $PATH_LOCAL"
    eval "scp -r $PATH_CLUSTER/slurm* $PATH_LOCAL"
    eval "scp -r $PATH_CLUSTER/outputs/ $PATH_LOCAL"
    eval "scp -r $PATH_CLUSTER/console/ $PATH_LOCAL"
    # eval "scp -r $PATH_CLUSTER/console_weighted/ $PATH_LOCAL"
}



# Clean code outputs
clean_code() {
    reload_utils

    rm -r ./outputs/solutions/*
    rm -r ./outputs/reports/*
    rm -r ./outputs/results/*
    rm model.lp
}

# Restart code configuration
restart_code_config() {
    rm -r build/
    mkdir build && cd build/ && cmake -DUSE_CLUSTER=ON .. && make -j && cd ..
}

# Recompile code
recompile_code() {
    cd build/ && make -j && cd ..
}

# Test scripts
test_literature() {
    bash ./scripts/test_lit.sh "$1" "$2"
}

test_random_grid() {
    bash ./scripts/test_random_grid.sh
}

test_paca() {
    bash ./scripts/test_paca.sh
}

# Slurm test scripts
slurm_test_literature() {
    sbatch ./scripts/test_lit.sh
}

slurm_test_random_grid() {
    sbatch ./scripts/test_random_grid.sh
}

slurm_test_paca() {
    sbatch ./scripts/test_paca.sh
}

# Create list of instances for literature
create_list_instances() {
    reload_utils

    local entries=("$@")
    local filename=./scripts/filter_lit.txt

    printf "%s\n" "${entries[@]}" > "$filename"
    echo "List generated successfully in $filename"
}

create_list_all_instances_literature() {
    local entries=(
        "SJC1" "SJC2" "SJC3a" "SJC3b" "SJC4a" "SJC4b" "p3038_1000" "p3038_600"
        "p3038_700" "p3038_800" "p3038_900" "spain737_148_1.txt" "spain737_148_2.txt"
        "spain737_74_1.txt" "spain737_74_2.txt" "ali535_005.txt" "ali535_025.txt"
        "ali535_050.txt" "ali535_100.txt" "ali535_150.txt" "fnl4461_0020.txt"
        "fnl4461_0100.txt" "fnl4461_0250.txt" "fnl4461_0500.txt" "fnl4461_1000.txt"
        "lin318_005.txt" "lin318_015.txt" "lin318_040.txt" "lin318_070.txt"
        "lin318_100.txt" "pr2392_020.txt" "pr2392_075.txt" "pr2392_150.txt"
        "pr2392_300.txt" "pr2392_500.txt" "rl1304_010.txt" "rl1304_050.txt"
        "rl1304_100.txt" "rl1304_200.txt" "rl1304_300.txt" "u724_010.txt"
        "u724_030.txt" "u724_075.txt" "u724_125.txt" "u724_200.txt" "FNA52057_1000.txt"
        "FNA52057_100.txt" "FNA52057_2000.txt" "LRA498378_1000.txt" "LRA498378_100.txt"
        "LRA498378_2000.txt" "SRA104814_1000.txt" "SRA104814_100.txt" "SRA104814_2000.txt"
        "XMC10150_1000.txt" "XMC10150_100.txt" "XMC10150_2000.txt"
    )

    create_list_instances "${entries[@]}"
}

create_list_pr2392_instances_literature() {
    local entries=(
        "pr2392_020.txt" "pr2392_075.txt" "pr2392_150.txt" "pr2392_300.txt" "pr2392_500.txt"
    )

    create_list_instances "${entries[@]}"
}

create_list_SJC_instances_literature() {
    local entries=(
        "SJC1" "SJC2" "SJC3a" "SJC3b" "SJC4a" "SJC4b"
    )

    create_list_instances "${entries[@]}"
}