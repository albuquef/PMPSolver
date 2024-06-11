
# Path: scripts/send_code_to_cluster.sh
# Compare this snippet from scripts/send_code_to_cluster.sh:

DATE=$(date +"%y-%m-%d")
# TYPEINSTANCE=Random_Grid
TYPEINSTANCE=PACA_instance

PATH_CLUSTER=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/Large_Instances/
# PATH_CLUSTER=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/test_all_coverages/
# PATH_CLUSTER=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/${TYPEINSTANCE}/
# PATH_LOCAL=/home/falbuquerque/Documents/projects/save_old_test_cluster/${DATE}_save_cluster/${TYPEINSTANCE}/
PATH_LOCAL=/home/falbuquerque/Documents/projects/Project_PMP/saves/SaveCluster/savecluster_Literature/2024-06-07_save-cluster/


# Save the outputs to the local machine
# eval "scp -r $PATH_CLUSTER $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/slurm* $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/solutions/ $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/reports/ $PATH_LOCAL"
# eval "scp -r $PATH_CLUSTER/console/ $PATH_LOCAL"
