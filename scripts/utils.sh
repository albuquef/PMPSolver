#!/bin/bash

# source ./scripts/utils.sh to use the functions

DATE=$(date +"%y-%m-%d")

MACHINE_CLUSTER=poseidon
USER_CLUSTER=falbuquerque
CLUSTER_SERVER="${USER_CLUSTER}@${MACHINE_CLUSTER}"

BASE_PATH="/home/falbuquerque/Documents/projects/Project_PMP/large-PMP"
# BASE_PATH="~/Documents/Projects/GeoAvigon/pmp_code/large-PMP"


reload_utils() {
    # add permissions to the file
    # chmod +x ./scripts/utils.sh
    # bash ./scripts/utils.sh
    source ./scripts/utils.sh
    echo "Reloaded utils.sh"
}

send_code_to_cluster() {

    # call the reload function
    reload_utils


    if [ -z "$1" ]; then
        path="/"
    else
        path=$1
    fi
    # path="/"
    # path="/src/"
    # path="/scripts/"


    PATH_ORIGIN=${BASE_PATH}/${path}/

    DIR_DEST=test_lit
    # PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/Benchmark_Lit${path}
    PATH_DEST="${CLUSTER_SERVER}:/users/falbuquerque/Projects/Project_PMP/${DIR_DEST}/${path}"

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

    DIR_NAME=test_code   
    echo "DIR_NAME: $DIR_NAME"
    PATH_CLUSTER=${CLUSTER_SERVER}:/users/falbuquerque/Projects/Project_PMP/${DIR_NAME}/

    PATH_LOCAL=~/Documents/projects/Project_PMP/saves/SaveCluster/${DATE}_save_cluster/${DIR_NAME}
    # PATH_LOCAL=~/Documents/Projects/GeoAvigon/save_cluster/${DATE}_save_cluster/
    mkdir -p $PATH_LOCAL

    # Save the outputs to the local machine
    # eval "scp -r $PATH_CLUSTER $PATH_LOCAL"
    eval "scp -r $PATH_CLUSTER/slurm* $PATH_LOCAL"
    eval "scp -r $PATH_CLUSTER/outputs/ $PATH_LOCAL"
    eval "scp -r $PATH_CLUSTER/console/ $PATH_LOCAL"
    # eval "scp -r $PATH_CLUSTER/console_weighted/ $PATH_LOCAL"
}


clean_code () {

    # call the reload function
    reload_utils

    # Clean the code
    # eval "rm -r build/"
    eval "rm -r ./outputs/solutions/*"
    eval "rm -r ./outputs/reports/*"
    eval "rm -r ./outputs/results/*"
    eval "rm model.lp"
}

restart_code_config () {
    # Clean the code
    eval "rm -r build/"
    # Configure the code again
    eval "mkdir build && cd build/ && cmake -DUSE_CLUSTER=ON .. && make -j && cd .."
}
recompile_code () {
    # Recompile the code
    eval "cd build/ && make -j && cd .."
}

test_literature() {
    # Run the code
    eval "bash ./scripts/test_lit.sh" $1 $2
}
test_random_grid() {
    # Run the code
    eval "bash ./scripts/test_random_grid.sh"
}
test_paca() {
    # Run the code
    eval "bash ./scripts/test_paca.sh"
}

slurm_test_literature() {
    # Run the code
    eval "sbatch ./scripts/test_lit.sh"
}
slurm_test_random_grid() {
    # Run the code
    eval "sbatch ./scripts/test_random_grid.sh"
}
slurm_test_paca() {
    # Run the code
    eval "sbatch ./scripts/test_paca.sh"
}

create_list_all_instances_literature() {

    # call the reload function
    reload_utils

    # Array of all the entries
    entries=(
        "SJC1"
        "SJC2"
        "SJC3a"
        "SJC3b"
        "SJC4a"
        "SJC4b"
        "p3038_1000"
        "p3038_600"
        "p3038_700"
        "p3038_800"
        "p3038_900"
        "spain737_148_1.txt"
        "spain737_148_2.txt"
        "spain737_74_1.txt"
        "spain737_74_2.txt"
        "ali535_005.txt"
        "ali535_025.txt"
        "ali535_050.txt"
        "ali535_100.txt"
        "ali535_150.txt"
        "fnl4461_0020.txt"
        "fnl4461_0100.txt"
        "fnl4461_0250.txt"
        "fnl4461_0500.txt"
        "fnl4461_1000.txt"
        "lin318_005.txt"
        "lin318_015.txt"
        "lin318_040.txt"
        "lin318_070.txt"
        "lin318_100.txt"
        "pr2392_020.txt"
        "pr2392_075.txt"
        "pr2392_150.txt"
        "pr2392_300.txt"
        "pr2392_500.txt"
        "rl1304_010.txt"
        "rl1304_050.txt"
        "rl1304_100.txt"
        "rl1304_200.txt"
        "rl1304_300.txt"
        "u724_010.txt"
        "u724_030.txt"
        "u724_075.txt"
        "u724_125.txt"
        "u724_200.txt"
        "FNA52057_1000.txt"
        "FNA52057_100.txt"
        "FNA52057_2000.txt"
        "LRA498378_1000.txt"
        "LRA498378_100.txt"
        "LRA498378_2000.txt"
        "SRA104814_1000.txt"
        "SRA104814_100.txt"
        "SRA104814_2000.txt"
        "XMC10150_1000.txt"
        "XMC10150_100.txt"
        "XMC10150_2000.txt"
    )

    filename=./scripts/filter_lit.txt
    # Write entries to output file
    printf "%s\n" "${entries[@]}" > $filename

    echo "List generated successfully in $filename"



}


create_list_pr2392_instances_literature() {

    # call the reload function
    reload_utils

    # Array of all the entries
    entries=(
        "pr2392_020.txt"
        "pr2392_075.txt"
        "pr2392_150.txt"
        "pr2392_300.txt"
        "pr2392_500.txt"
    )

    filename=./scripts/filter_lit.txt
    # Write entries to output file
    printf "%s\n" "${entries[@]}" > $filename

    echo "List generated successfully in $filename"



}

create_list_SJC_instances_literature() {

    # call the reload function
    reload_utils

    # Array of all the entries
    entries=(
        "SJC1"
        "SJC2"
        "SJC3a"
        "SJC3b"
        "SJC4a"
        "SJC4b"
    )

    filename=./scripts/filter_lit.txt
    # Write entries to output file
    printf "%s\n" "${entries[@]}" > $filename

    echo "List generated successfully in $filename"



}