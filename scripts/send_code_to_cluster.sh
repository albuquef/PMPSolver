
# path="/"
# path="/src/"
path="/scripts/"
# path="/data/"

# PATH_ORIGIN=/home/felipe/Documents/Projects/GeoAvigon/pmp_code/large-PMP${path}
PATH_ORIGIN=/home/falbuquerque/Documents/projects/Project_PMP/large-PMP${path}
PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/cpmp/test_exact${path}
# PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/Benchmark_Lit${path}
# PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/Random_Grid${path}



# Copy the code to the cluster
eval "rsync -av --exclude='.git' $PATH_ORIGIN $PATH_DEST"

# Copy the data to the cluster
# eval "rsync -av $PATH_ORIGIN/data/filterData_PACA_may23/ $PATH_DEST/data/filterData_PACA_may23/"


#  rm -r build/ 
#  mkdir build && cd build/ && cmake -DUSE_CLUSTER=ON .. && make -j && cd ..