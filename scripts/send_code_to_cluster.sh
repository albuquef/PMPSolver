
path="/"
# path="/src/"
# path="/scripts/"
# path="/data/"


BASE_PATH="/home/felipe/Documents/Projects/GeoAvigon/pmp_code/large-PMP"
# BASE_PATH="/home/falbuquerque/Documents/projects/Project_PMP/large-PMP"

# PATH_ORIGIN=/home/felipe/Documents/Projects/GeoAvigon/pmp_code/large-PMP${path}
PATH_ORIGIN=${BASE_PATH}${path}
# PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/cpmp_cover/test_coverages_exact${path}
# PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/multiechelle${path}
# PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/Benchmark_Lit${path}
# PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/Random_Grid${path}

PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/test_all_coverages${path}


# Copy the code to the cluster
eval "rsync -av --exclude='.git' $PATH_ORIGIN $PATH_DEST"

# Copy the data to the cluster
# eval "rsync -av $PATH_ORIGIN/data/filterData_PACA_may23/ $PATH_DEST/data/filterData_PACA_may23/"


# rm -r build/ 
# mkdir build && cd build/ && cmake -DUSE_CLUSTER=ON .. && make -j && cd ..