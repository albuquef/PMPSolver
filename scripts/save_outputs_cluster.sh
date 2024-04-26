
# Path: scripts/send_code_to_cluster.sh
# Compare this snippet from scripts/send_code_to_cluster.sh:

DATE=$(date +"%y-%m-%d")
# TYPEINSTANCE=Random_Grid
TYPEINSTANCE=PACA_instance



PATH_CLUSTER=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/cpmp/test_exact
# PATH_CLUSTER=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/${TYPEINSTANCE}/
PATH_LOCAL=/home/falbuquerque/Documents/projects/save_old_test_cluster/${DATE}_save_cluster/${TYPEINSTANCE}/


# Save the outputs to the local machine
eval "scp -r $PATH_CLUSTER/slurm* $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/solutions/ $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/reports/ $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/console/ $PATH_LOCAL"
