#include "solver_config.hpp"


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
            }else if (key == "--mode") {
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
            }else if (key == "-time_subprob_rssv") {
                config.CLOCK_LIMIT_SUBPROB_RSSV = std::stod(argv[i+1]);
                configOverride.insert("time_subprob_rssv");
            }else if (key == "-max_ite_subprob_rssv") {
                config.MAX_ITE_SUBPROB_RSSV = std::stoi(argv[i+1]);
                configOverride.insert("max_ite_subprob_rssv");
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
            }else if (key == "-bw_multiplier") {
                config.BW_MULTIPLIER = std::stod(argv[i+1]);
                configOverride.insert("bw_multiplier");
                BW_MULTIPLIER = config.BW_MULTIPLIER;
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
                    throw std::invalid_argument("Unknown parameter [cover mode n2]: " + std::string(argv[i+1]));
                }
                configOverride.insert("cover_mode_n2");
            } else if (key == "-add_threshold_distance_rssv") {
                if (strcmp(argv[i+1], "true") == 0 || strcmp(argv[i+1], "1") == 0) {
                    config.add_threshold_distance_rssv = true;
                } else if (strcmp(argv[i+1], "false") == 0 || strcmp(argv[i+1], "0") == 0) {
                    config.add_threshold_distance_rssv = false;
                } else {
                    throw std::invalid_argument("Unknown parameter [add_threshold_distance_rssv]: " + std::string(argv[i+1]));
                }
                configOverride.insert("add_threshold_distance_rssv");
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
    configParser.setFromConfig(&config.add_threshold_distance_rssv, "add_threshold_distance_rssv");
    configParser.setFromConfig(&config.MAX_ITE_SUBPROB_RSSV, "max_ite_subprob_rssv");
    configParser.setFromConfig(&config.CLOCK_LIMIT_SUBPROB_RSSV, "time_subprob_rssv");
    configParser.setFromConfig(&config.BW_MULTIPLIER, "bw_multiplier");

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

