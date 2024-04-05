PATH_ORIGIN=/home/falbuquerque/Documents/projects/Project_PMP/large-PMP/src/
PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/cpmp/test_exact/src/
# PATH_DEST=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/test_code/src/



# Copy the code to the cluster
eval "rsync -av --exclude='.git' $PATH_ORIGIN $PATH_DEST"

# Copy the data to the cluster
# eval "rsync -av $PATH_ORIGIN/data/filterData_PACA_may23/ $PATH_DEST/data/filterData_PACA_may23/"


#  rm -r build/ 
#  mkdir build && cd build/ && cmake -DUSE_CLUSTER=ON .. && make -j && cd ..