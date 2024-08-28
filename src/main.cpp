#include <set>
#include <cstring>
#include <string>
#include <chrono> // for time-related functions
using namespace std;
using namespace chrono;


#include "globals.hpp"
#include "solver_config.hpp" // #include "config_parser.cpp"
#include "solver_manager.hpp"
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"
#include "TBPercentage.hpp"
#include "utils.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "solution_map.hpp"
#include "PMP.hpp"
#include "VNS.hpp"
#include "post_optimization.hpp"

Instance setupInstance(const Config& config) {
    cout << "Loading instance...\n";
    
    // Load instance
    Instance instance(config.dist_matrix_filename, 
                      config.labeled_weights_filename, 
                      config.capacities_filename, 
                      config.p, ' ', 
                      config.TypeService, 
                      config.cust_max_id, 
                      config.loc_max_id);

    if (!config.coverages_filename.empty() && config.cover_mode) {
        instance.ReadCoverages(config.coverages_filename, config.TypeSubarea, ' ');
        instance.setCoverModel(true);
    }

    if (!config.coverages_filename_n2.empty() && config.cover_mode_n2) {
        instance.ReadCoverages_n2(config.coverages_filename_n2, config.TypeSubarea_n2, ' ');
        instance.setCoverModel_n2(true);
    }

    cout << "[INFO] Instance loaded\n";
    instance.set_isWeightedObjFunc(config.IsWeighted_ObjFunc);
    if (config.fixed_threshold_distance > 0){
        instance.set_ThresholdDist(dist_t(config.fixed_threshold_distance));
    }
    instance.print();

    return instance;
}

// Solution_MAP solution_map;
// Solution_std methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
// Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
// void solveProblem(const Instance& instance, const Config& config, int seed);


int main(int argc, char *argv[]) {

    Config config;
    // set<const char*> configOverride;
    std::set<std::string> configOverride;
    try {
        parseArguments(argc, argv, config);
        setupConfig(config, config.configOverride);
    } catch (const std::exception& e) {
        std::cerr << "Error parse arguments/setup config: " << e.what() << std::endl;
        return 1;
    }
    // Check required parameters before proceeding
    checkRequiredParameters(config);
    setThreadNumber(config.threads_cnt);

    Instance instance = setupInstance(config);

    // filter instance
    // Instance instance_original = setupInstance(config); // Instance instance = instance_original.filterInstance(TypeService);

    auto start = tick();
    SolverManager solverManager(config, instance);
    solverManager.solveProblem();
    tock(start);



    // auto start = tick();
    // solveProblem(instance, config, config.seed);
    // cout << endl;
    // tock(start);

    return 0;
}


