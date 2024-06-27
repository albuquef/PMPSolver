#include <set>
#include <cstring>
#include <string>
#include <chrono> // for time-related functions
using namespace std;
using namespace std::chrono;

#include "globals.hpp"
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"
#include "TBPercentage.hpp"
#include "utils.hpp"
#include "config_parser.cpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "solution_map.hpp"
#include "PMP.hpp"
#include "VNS.hpp"

struct Config {
    // Required parameters
    uint_t p = 0;
    string dist_matrix_filename;
    string labeled_weights_filename;
    string capacities_filename;
    string TypeService;
    string coverages_filename;
    string coverages_filename_n2;
    string TypeSubarea;
    string TypeSubarea_n2;

    // Optional parameters with default values
    uint_t threads_cnt = (uint_t) getAvailableThreads();  // Default initialized based on available threads
    int mode = 0;
    int seed = 1;
    string output_filename;
    string Method;
    string Method_RSSV_sp;
    string Method_RSSV_fp;
    bool VERBOSE = false;
    uint_t TOLERANCE_CPT = 10;
    uint_t K = 1;
    uint_t PERCENTAGE = 0;
    double CLOCK_LIMIT = 0;
    double CLOCK_LIMIT_CPLEX = 0;

    bool cover_mode = false;
    bool cover_mode_n2 = false;
    uint_t cust_max_id = 0;
    uint_t loc_max_id = 0;
    uint_t size_subproblems_rssv = 800;
    bool IsWeighted_ObjFunc = false;
    set<const char*> configOverride;
    string configPath = "config.toml";
};

double get_wall_time_main(){
    struct timeval time;
    if(gettimeofday(&time,nullptr)){
        // HANDLE ERROR
        return 0;
    }else{
        return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_usec*0.000001); //microsegundos
    }
}
void parseArguments(int argc, char* argv[], Config& config) {
    std::set<const char*> configOverride;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' || argv[i][0] == '?') {
            std::string key = argv[i];

            if (key == "-p") {
                config.p = std::stoi(argv[i+1]);
                configOverride.insert("p");
            } else if (key == "-v" || key == "--verbose") {
                config.VERBOSE = true;
                configOverride.insert("verbose");
            } else if (key == "-config") {
                config.configPath = argv[i+1];
            } else if (key == "-dm") {
                config.dist_matrix_filename = argv[i+1];
                configOverride.insert("distance_matrix");
            } else if (key == "-w") {
                config.labeled_weights_filename = argv[i+1];
                configOverride.insert("weights");
            } else if (key == "-th") {
                config.threads_cnt = std::stoi(argv[i+1]);
                configOverride.insert("threads");
            } else if (key == "-cust_max_id") {
                config.cust_max_id = std::stoi(argv[i+1]);
                configOverride.insert("cust_max_id");
            }else if (key == "-loc_max_id") {
                config.loc_max_id = std::stoi(argv[i+1]);
                configOverride.insert("loc_max_id");
            } else if (key == "--mode") {
                config.mode = std::stoi(argv[i+1]);
                configOverride.insert("mode");
            } else if (key == "-IsWeighted_ObjFunc") {
                if (strcmp(argv[i+1], "true") == 0 || strcmp(argv[i+1], "1") == 0) {
                    config.IsWeighted_ObjFunc = true;
                } else if (strcmp(argv[i+1], "false") == 0 || strcmp(argv[i+1], "0") == 0) {
                    config.IsWeighted_ObjFunc = false;
                } else {
                    throw std::invalid_argument("Unknown parameter [IsWeighted_ObjFunc]: " + std::string(argv[i+1]));
                }
                configOverride.insert("IsWeighted_ObjFunc");
            } else if (key == "--seed") {
                config.seed = std::stoi(argv[i+1]);
                configOverride.insert("seed");
            } else if (key == "-t" || key == "-time") {
                config.CLOCK_LIMIT = std::stod(argv[i+1]);
                configOverride.insert("time");
            } else if (key == "-time_cplex") {
                config.CLOCK_LIMIT_CPLEX = std::stod(argv[i+1]);
                configOverride.insert("time_cplex");
            } else if (key == "-o") {
                config.output_filename = argv[i+1];
                configOverride.insert("output");
            } else if (key == "-c") {
                config.capacities_filename = argv[i+1];
                configOverride.insert("capacities");
            } else if (key == "-cover") {
                config.coverages_filename = argv[i+1];
                configOverride.insert("coverages");
            } else if (key == "-cover_n2") {
                config.coverages_filename_n2 = argv[i+1];
                configOverride.insert("coverages_n2");
            } else if (key == "-toleranceCpt") {
                config.TOLERANCE_CPT = std::stoi(argv[i+1]);
                configOverride.insert("toleranceCpt");
            } else if (key == "-k") {
                config.K = std::stoi(argv[i+1]);
                configOverride.insert("k");
            } else if (key == "-percentage") {
                config.PERCENTAGE = std::stoi(argv[i+1]);
                configOverride.insert("percentage");
            } else if (key == "-service") {
                config.TypeService = argv[i+1];
                configOverride.insert("service");
            } else if (key == "-subarea") {
                config.TypeSubarea = argv[i+1];
                configOverride.insert("subarea");
            } else if (key == "-subarea_n2") {
                config.TypeSubarea_n2 = argv[i+1];
                configOverride.insert("subarea_n2");
            } else if (key == "-method") {
                config.Method = argv[i+1];
                configOverride.insert("method");
            } else if (key == "-method_rssv_sp") {
                config.Method_RSSV_sp = argv[i+1];
                configOverride.insert("method_rssv_sp");
            } else if (key == "-method_rssv_fp") {
                config.Method_RSSV_fp = argv[i+1];
                configOverride.insert("method_rssv_fp");
            } else if (key == "-size_subproblems_rssv") {
                config.size_subproblems_rssv = std::stoi(argv[i+1]);
                configOverride.insert("size_subproblems_rssv");
                SUB_PMP_SIZE = static_cast<uint_t>(config.size_subproblems_rssv);
            } else if (key == "-cover_mode") {
                if (strcmp(argv[i+1], "true") == 0 || strcmp(argv[i+1], "1") == 0) {
                    config.cover_mode = true;
                } else if (strcmp(argv[i+1], "false") == 0 || strcmp(argv[i+1], "0") == 0) {
                    config.cover_mode = false;
                } else {
                    throw std::invalid_argument("Unknown parameter [cover mode]: " + std::string(argv[i+1]));
                }
                configOverride.insert("cover_mode");
            } else if (key == "-cover_mode_n2") {
                if (strcmp(argv[i+1], "true") == 0 || strcmp(argv[i+1], "1") == 0) {
                    config.cover_mode_n2 = true;
                } else if (strcmp(argv[i+1], "false") == 0 || strcmp(argv[i+1], "0") == 0) {
                    config.cover_mode_n2 = false;
                } else {
                    throw std::invalid_argument("Unknown parameter [cover mode]: " + std::string(argv[i+1]));
                }
                configOverride.insert("cover_mode_n2");
            } else if (key == "--help" || key == "-h" || key == "?") {
                std::cout << "Usage instructions:\n";
                std::cout << "-p <value>          : Number of medians to select.\n";
                std::cout << "-dm <filename>      : Path to the distance matrix file.\n";
                std::cout << "-w <filename>       : Path to the labeled weights file.\n";
                // Add other parameter descriptions...
                exit(0);
            } else {
                throw std::invalid_argument("Unknown parameter: " + key);
            }
        }
    }
    config.configOverride = configOverride;
}
void setupConfig(Config& config, std::set<const char*>& configOverride) {

    ConfigParser configParser(config.configPath, configOverride);

    configParser.setFromConfig(&config.VERBOSE, "verbose");
    configParser.setFromConfig(&config.p, "p");
    configParser.setFromConfig(&config.capacities_filename, "capacities");
    configParser.setFromConfig(&config.coverages_filename, "coverages");
    configParser.setFromConfig(&config.dist_matrix_filename, "distance_matrix");
    configParser.setFromConfig(&config.output_filename, "output");
    configParser.setFromConfig(&config.labeled_weights_filename, "weights");
    configParser.setFromConfig(&config.threads_cnt, "threads");
    configParser.setFromConfig(&config.mode, "mode");
    configParser.setFromConfig(&config.cust_max_id, "cust_max_id");
    configParser.setFromConfig(&config.loc_max_id, "loc_max_id");
    configParser.setFromConfig(&config.seed, "seed");
    configParser.setFromConfig(&config.CLOCK_LIMIT, "time");
    configParser.setFromConfig(&config.CLOCK_LIMIT_CPLEX, "time_cplex");
    configParser.setFromConfig(&config.TOLERANCE_CPT, "toleranceCpt");
    configParser.setFromConfig(&config.K, "k");
    configParser.setFromConfig(&config.PERCENTAGE, "percentage");
    configParser.setFromConfig(&config.TypeService, "service");
    configParser.setFromConfig(&config.Method, "method");
    configParser.setFromConfig(&config.Method_RSSV_sp, "method_rssv_sp");
    configParser.setFromConfig(&config.Method_RSSV_fp, "method_rssv_fp");
    configParser.setFromConfig(&config.coverages_filename_n2, "coverages_n2");
    configParser.setFromConfig(&config.size_subproblems_rssv, "size_subproblems_rssv");
    configParser.setFromConfig(&config.TypeSubarea, "subarea");
    configParser.setFromConfig(&config.TypeSubarea_n2, "subarea_n2");
    configParser.setFromConfig(&config.cover_mode, "cover_mode");
    configParser.setFromConfig(&config.cover_mode_n2, "cover_mode_n2");
    configParser.setFromConfig(&config.IsWeighted_ObjFunc, "IsWeighted_ObjFunc");

    // Additional fields can be set similarly

    // Debugging: Print the contents of configOverride
    std::cout << "Debug: configOverride contents: ";
    for (const auto& key : configOverride) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
}
void checkRequiredParameters(const Config& config) {
    if (config.p == 0) {
        std::cerr << "Error: No. of medians (-p) not provided.\n";
        exit(1);
    }
    if (config.dist_matrix_filename.empty()) {
        std::cerr << "Error: Distance matrix (-dm) not provided.\n";
        exit(1);
    }
    if (config.labeled_weights_filename.empty()) {
        std::cerr << "Error: Customer weights (-w) not provided.\n";
        exit(1);
    }
    if (config.capacities_filename.empty()) {
        std::cerr << "Error: Location capacities (-c) not provided.\n";
        exit(1);
    }
    // Add more checks for other required parameters as needed
}

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
    instance.print();

    return instance;
}

Solution_MAP solution_map;
Solution_std methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
void solveProblem(const Instance& instance, const Config& config, int seed);


int main(int argc, char *argv[]) {

    Config config;
    set<const char*> configOverride;
    try {
        parseArguments(argc, argv, config);
        setupConfig(config, config.configOverride);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    // Check required parameters before proceeding
    checkRequiredParameters(config);
    setThreadNumber(config.threads_cnt);

    Instance instance = setupInstance(config);

    // filter instance
    // Instance instance_original = setupInstance(config); // Instance instance = instance_original.filterInstance(TypeService);

    auto start = tick();
    solveProblem(instance, config, config.seed);
    cout << endl;
    tock(start);

    return 0;
}


void solveProblem(const Instance& instance, const Config& config, int seed) {
    cout << "-------------------------------------------------\n";
    
    if (config.Method == "EXACT_PMP" || config.Method == "TB_PMP" || config.Method == "VNS_PMP") {
        auto start_time = high_resolution_clock::now();
        Solution_std solution = methods_PMP(make_shared<Instance>(instance), config, 0);
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
        
        cout << "\nFinal solution std:\n";
        solution.print();
        solution.statsDistances();
        cout << "Statistics:\n";
        cout << "Max distance: " << solution.getMaxDist() << endl;
        cout << "Min distance: " << solution.getMinDist() << endl;
        cout << "Avg distance: " << solution.getAvgDist() << endl;
        cout << "Std deviation distance: " << solution.getStdDevDist() << endl;
        cout << endl << endl;
        cout << "Final total elapsed time: " << elapsed_time << "s\n";
        solution.saveAssignment(config.output_filename, config.Method);
        solution.saveResults(config.output_filename, elapsed_time, 0, config.Method);   
    } 
    else if (config.Method == "EXACT_CPMP" || config.Method == "EXACT_CPMP_BIN" || config.Method == "TB_CPMP" || 
             config.Method == "VNS_CPMP" || config.Method == "GAPrelax" || config.Method == "GAP") {
        
        auto start_time = high_resolution_clock::now();
        Solution_cap solution = methods_CPMP(make_shared<Instance>(instance), config, 0);
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();

        cout << "\nFinal solution:\n";
        solution.print();
        solution.statsDistances();
        cout << "Statistics:\n";
        cout << "Max distance: " << solution.getMaxDist() << endl;
        cout << "Min distance: " << solution.getMinDist() << endl;
        cout << "Avg distance: " << solution.getAvgDist() << endl;
        cout << "Std deviation distance: " << solution.getStdDevDist() << endl;
        cout << endl << endl;
        cout << "Final total elapsed time: " << elapsed_time << "s\n";  
        solution.saveAssignment(config.output_filename, config.Method);
        solution.saveResults(config.output_filename, elapsed_time, 0, config.Method); 
    } 
    else if (config.Method == "RSSV") {
        cout << "RSSV heuristic \n";
        cout << "-------------------------------------------------\n";
        
        RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
        metaheuristic.setCoverMode(config.cover_mode);
        metaheuristic.setCoverMode_n2(config.cover_mode_n2);
        CLOCK_THREADED = true;
        auto start_time_total = high_resolution_clock::now();

        shared_ptr<Instance> filtered_instance;
        if (config.Method_RSSV_sp == "EXACT_PMP" || config.Method_RSSV_sp == "TB_PMP" || config.Method_RSSV_sp == "VNS_PMP") {
            filtered_instance = metaheuristic.run(THREAD_NUMBER, config.Method_RSSV_sp);
        } 
        else if (config.Method_RSSV_sp == "EXACT_CPMP" || config.Method_RSSV_sp == "EXACT_CPMP_BIN" || 
                 config.Method_RSSV_sp == "TB_CPMP" || config.Method_RSSV_sp == "VNS_CPMP") {
            filtered_instance = metaheuristic.run_CAP(THREAD_NUMBER, config.Method_RSSV_sp);
        } 
        else {
            cerr << "[ERROR] Method subproblem RSSV not found" << endl;
            exit(1);
        }

        filtered_instance->setCoverModel(config.cover_mode);
        filtered_instance->setCoverModel_n2(config.cover_mode_n2);
        filtered_instance->set_isWeightedObjFunc(config.IsWeighted_ObjFunc);
        
        cout << "Final instance parameters:\n";
        filtered_instance->print();

        // exit(1);

        cout << "-------------------------------------------------\n";
        cout << "Final Problem RSSV heuristic \n";
        cout << "-------------------------------------------------\n";
        
        if (config.Method_RSSV_fp == "EXACT_PMP" || config.Method_RSSV_fp == "TB_PMP" || config.Method_RSSV_fp == "VNS_PMP") {
            auto start_time = high_resolution_clock::now();
            Solution_std solution = methods_PMP(filtered_instance, config,
                                    duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();

            cout << "\nFinal solution std:\n";
            solution.print();
            cout << "Final problem elapsed time: " << elapsed_time << "s\n";
            cout << "Final total elapsed time: " << duration_cast<seconds>(current_time - start_time_total).count() << "s\n";
            solution.saveAssignment(config.output_filename, config.Method);
            solution.saveResults(config.output_filename, elapsed_time, 0, config.Method); 
        } 
        else if (config.Method_RSSV_fp == "EXACT_CPMP" || config.Method_RSSV_fp == "EXACT_CPMP_BIN" || 
                 config.Method_RSSV_fp == "TB_CPMP" || config.Method_RSSV_fp == "VNS_CPMP") {
            auto start_time = high_resolution_clock::now();        
            Solution_cap solution = methods_CPMP(filtered_instance, config,
                                                duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            
            if (config.Method_RSSV_fp == "EXACT_CPMP" || config.Method_RSSV_fp == "EXACT_CPMP_BIN") {
                solution.objEval();
            }

            cout << "\nFinal solution:\n";
            solution.print();
            cout << "Final elapsed time: " << elapsed_time << "s\n";
            cout << "Final total elapsed time: " << duration_cast<seconds>(current_time - start_time_total).count() << "s\n";
            solution.saveAssignment(config.output_filename, "RSSV_" + config.Method_RSSV_fp);
            solution.saveResults(config.output_filename, elapsed_time, 0, config.Method, config.Method_RSSV_sp, config.Method_RSSV_fp);
        }
    }
}



Solution_std methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time) {
    Solution_std solution;
    cout << "-------------------------------------------------\n";

    if (config.Method == "EXACT_PMP") {
        cout << "Exact method PMP\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "PMP");
        pmp.setCoverModel(config.cover_mode, instance->getTypeSubarea());
        pmp.setCoverModel_n2(config.cover_mode_n2, instance->getTypeSubarea_n2());
        pmp.run(config.Method);
        pmp.saveVars(config.output_filename, config.Method);
        pmp.saveResults(config.output_filename, config.Method);
        solution = pmp.getSolution_std();
    } else if (config.Method == "TB_PMP") {
        cout << "TB heuristic - standard PMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, config.seed);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setTimeLimit(config.CLOCK_LIMIT);
        solution = heuristic.run(true, UB_MAX_ITER);
    } else if (config.Method == "VNS_PMP") {
        cout << "VNS heuristic - PMP\n";
        cout << "-------------------------------------------------\n";
        VNS heuristic(instance, config.seed);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setExternalTime(external_time); // external time is not used in PMP
        solution = heuristic.runVNS_std(true, UB_MAX_ITER);
    } else {
        cout << "[ERROR] Method not found" << endl;
        exit(1);
    }
    return solution;
}
Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time) {
    Solution_cap solution;
    Solution_MAP solution_map(instance);
    bool add_InitialSolution_RSSV = false;

    string Method = config.Method;
    if (Method == "RSSV") {
        Method = config.Method + "_" + config.Method_RSSV_fp;
    }


    cout << "-------------------------------------------------\n";
    if (Method == "EXACT_CPMP" || Method == "RSSV_EXACT_CPMP") {
        cout << "Exact method cPMP continuos\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "CPMP");
        pmp.setGenerateReports(true);
        pmp.setCoverModel(config.cover_mode, instance->getTypeSubarea());
        pmp.setCoverModel_n2(config.cover_mode_n2, instance->getTypeSubarea_n2());
        pmp.setTimeLimit(config.CLOCK_LIMIT_CPLEX - external_time);
        if (config.CLOCK_LIMIT_CPLEX == 0) {pmp.setTimeLimit(0);} // no time limit

        if (add_InitialSolution_RSSV && Method == "RSSV_EXACT_CPMP") {
            auto vet_locs = instance->getVotedLocs();
            unordered_set<uint_t> init_p(vet_locs.begin(), vet_locs.begin() + instance->get_p());
            Solution_cap init_sol(instance, init_p, "GAPrelax");
            if (init_sol.getFeasibility()) {
                pmp.setMIPStartSolution(init_sol);
                init_sol.print();
            } else {
                cout << "Initial solution not feasible\n";
            }
        }

        pmp.run(Method);
        pmp.saveVars(config.output_filename, Method);
        pmp.saveResults(config.output_filename, Method);
        solution = pmp.getSolution_cap();
    } else if (Method == "EXACT_CPMP_BIN" || Method == "RSSV_EXACT_CPMP_BIN") {
        cout << "Exact method cPMP binary\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "CPMP", true);
        pmp.setGenerateReports(true);
        pmp.setCoverModel(config.cover_mode, instance->getTypeSubarea());
        pmp.setCoverModel_n2(config.cover_mode_n2, instance->getTypeSubarea_n2());
        pmp.setTimeLimit(config.CLOCK_LIMIT_CPLEX - external_time);
        if (config.CLOCK_LIMIT_CPLEX == 0) {pmp.setTimeLimit(0);} // no time limit

        if (add_InitialSolution_RSSV && Method == "RSSV_EXACT_CPMP_BIN") {
            auto vet_locs = instance->getVotedLocs();
            unordered_set<uint_t> init_p(vet_locs.begin(), vet_locs.begin() + instance->get_p());
            Solution_cap init_sol(instance, init_p, "GAP");
            if (init_sol.getFeasibility()) {
                pmp.setMIPStartSolution(init_sol);
                init_sol.print();
            } else {
                cout << "Initial solution not feasible\n";
            }
        }

        pmp.run(Method);
        pmp.saveVars(config.output_filename, Method);
        pmp.saveResults(config.output_filename, Method);
        solution = pmp.getSolution_cap();
    } else if (Method == "TB_CPMP" || Method == "RSSV_TB_CPMP") {
        cout << "TB heuristic - cPMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, config.seed);
        heuristic.setSolutionMap(solution_map);
        heuristic.setGenerateReports(true);
        heuristic.setMethod(Method);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setTimeLimit(config.CLOCK_LIMIT);
        solution = heuristic.run_cap(true, UB_MAX_ITER);
    } else if (Method == "VNS_CPMP" || Method == "RSSV_VNS_CPMP") {
        cout << "VNS heuristic - cPMP\n";
        cout << "-------------------------------------------------\n";
        VNS heuristic(instance, config.seed);
        heuristic.setSolutionMap(solution_map);
        heuristic.setGenerateReports(true);
        heuristic.setMethod(Method);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setExternalTime(external_time);

        if (add_InitialSolution_RSSV && Method == "RSSV_VNS_CPMP") {
            auto vet_locs = instance->getVotedLocs();
            unordered_set<uint_t> init_p(vet_locs.begin(), vet_locs.begin() + instance->get_p());
            Solution_cap init_sol(instance, init_p, "GAP");
            if (init_sol.getFeasibility()) {
                heuristic.setInitialSolution(init_sol);
                init_sol.print();
            } else {
                cout << "Initial solution not feasible\n";
            }
        }

        solution = heuristic.runVNS_cap(Method, true, UB_MAX_ITER);

        if (solution.isSolutionFeasible()) {
            cout << "Solution feasible\n";
        } else {
            cout << "Solution not feasible\n";
        }

    } else if (Method == "GAPrelax" || Method == "GAP") {
        cout << Method << " - cPMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, config.seed);
        heuristic.setMethod(Method);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        auto solution = heuristic.fixedCapSolution(Method);
        cout << "Final solution:\n";
        solution.print();
        exit(1);

    } else {
        cout << "[ERROR] Method not found" << endl;
        exit(1);
    }

    if (Method != "EXACT_CPMP" && Method != "EXACT_CPMP_BIN" && Method != "RSSV_EXACT_CPMP" && Method != "RSSV_EXACT_CPMP_BIN" && Method != "GAPrelax" && Method != "GAP") {
        solution.setCoverMode(config.cover_mode);
        auto p_loc = solution.get_pLocations();
        auto sol_best = Solution_cap(instance, p_loc, "GAPrelax", config.cover_mode);
        return sol_best;
    }

    auto sol_best = solution;
    sol_best.setCoverMode(config.cover_mode);
    sol_best.setCoverMode_n2(config.cover_mode_n2);

    return sol_best;
}