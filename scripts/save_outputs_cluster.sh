# Path: scripts/send_code_to_cluster.sh
# Compare this snippet from scripts/send_code_to_cluster.sh:
# PATH_CLUSTER=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/cpmp/test_exact/
PATH_CLUSTER=falbuquerque@poseidon:/users/falbuquerque/Projects/Project_PMP/PACA_cyrille/originial_instance/cpmp_cover/test_coverages_exact/
# PATH_LOCAL=//home/falbuquerque/Documents/projects/Project_PMP/Cluster_tests_saved/PACA_cyrille/original_instance/CPMP/test_exats_complement/
PATH_LOCAL=/home/felipe/Documents/Projects/GeoAvigon/save_cluster/test_coverages_exact/
# PATH_LOCAL=//home/falbuquerque/Documents/projects/Project_PMP/Cluster_tests_saved/PACA_cyrille/original_instance/CPMP_cover/test_coverages_exact/


# Save the outputs to the local machine
eval "scp -r $PATH_CLUSTER/slurm* $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/solutions/ $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/reports/ $PATH_LOCAL"
eval "scp -r $PATH_CLUSTER/console/ $PATH_LOCAL"
