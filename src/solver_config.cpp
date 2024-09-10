#include "solver_config.hpp"

void setBoolConfigOption(bool &configOption, const char* value, const std::string &paramName, std::set<std::string> &configOverride) {
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
        configOption = true;
    } else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
        configOption = false;
    } else {
        throw std::invalid_argument("Unknown parameter [" + paramName + "]: " + std::string(value));
    }
    configOverride.insert(paramName);
}


void parseArguments(int argc, char* argv[], Config& config) {
    // std::set<const char*> configOverride;
    
    std::set<std::string> configOverride;

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
                setBoolConfigOption(config.IsWeighted_ObjFunc, argv[i+1], "IsWeighted_ObjFunc", configOverride);
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
            } else if (key == "-size_final_prob_rssv") {
                config.size_final_prob_rssv = std::stoi(argv[i+1]);
                configOverride.insert("size_final_prob_rssv");
                FINAL_PROB_RSSV_SIZE = static_cast<uint_t>(config.size_final_prob_rssv);
            }else if (key == "-method_post_opt") {
                config.Method_PostOpt = argv[i+1];
                configOverride.insert("method_post_opt");
            } else if (key == "-bw_multiplier") {
                config.BW_MULTIPLIER = std::stod(argv[i+1]);
                configOverride.insert("bw_multiplier");
                BW_MULTIPLIER = config.BW_MULTIPLIER;
            } else if (key == "-fixed_threshold_distance") {
                config.fixed_threshold_distance = std::stod(argv[i+1]);
                configOverride.insert("fixed_threshold_distance");
            } else if (key == "-cover_mode") {
                setBoolConfigOption(config.cover_mode, argv[i+1], "cover_mode", configOverride);
            } else if (key == "-cover_mode_n2") {
                setBoolConfigOption(config.cover_mode_n2, argv[i+1], "cover_mode_n2", configOverride);
            } else if (key == "-add_threshold_distance_rssv") {
                setBoolConfigOption(config.add_threshold_distance_rssv, argv[i+1], "add_threshold_distance_rssv", configOverride);
            } else if (key == "-add_generate_reports") {
                setBoolConfigOption(config.add_generate_reports, argv[i+1], "add_generate_reports", configOverride);
            } else if (key == "-add_break_callback") {
                setBoolConfigOption(config.add_break_callback, argv[i+1], "add_break_callback", configOverride);
            }else if (key == "--help" || key == "-h" || key == "?") {
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

void setupConfig(Config& config, std::set<std::string>& configOverride) {

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
    configParser.setFromConfig(&config.size_final_prob_rssv, "size_final_prob_rssv");
    configParser.setFromConfig(&config.TypeSubarea, "subarea");
    configParser.setFromConfig(&config.TypeSubarea_n2, "subarea_n2");
    configParser.setFromConfig(&config.cover_mode, "cover_mode");
    configParser.setFromConfig(&config.cover_mode_n2, "cover_mode_n2");
    configParser.setFromConfig(&config.IsWeighted_ObjFunc, "IsWeighted_ObjFunc");
    configParser.setFromConfig(&config.add_threshold_distance_rssv, "add_threshold_distance_rssv");
    configParser.setFromConfig(&config.MAX_ITE_SUBPROB_RSSV, "max_ite_subprob_rssv");
    configParser.setFromConfig(&config.CLOCK_LIMIT_SUBPROB_RSSV, "time_subprob_rssv");
    configParser.setFromConfig(&config.BW_MULTIPLIER, "bw_multiplier");
    configParser.setFromConfig(&config.Method_PostOpt, "method_post_opt");
    configParser.setFromConfig(&config.add_generate_reports, "add_generate_reports");
    configParser.setFromConfig(&config.add_break_callback, "add_break_callback");
    configParser.setFromConfig(&config.fixed_threshold_distance, "fixed_threshold_distance");

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


// void ConfigParser::printINFOparameters(const Config& config) {
//     cout << " -------------------------------------------------- " << endl;
//     std::cout << "Parameters:\n";
//     std::cout << "p: " << config.p << std::endl;
//     std::cout << "dist_matrix_filename: " << config.dist_matrix_filename << std::endl;
//     std::cout << "labeled_weights_filename: " << config.labeled_weights_filename << std::endl;
//     std::cout << "capacities_filename: " << config.capacities_filename << std::endl;
//     std::cout << "TypeService: " << config.TypeService << std::endl;
//     std::cout << "IsWeighted_ObjFunc: " << config.IsWeighted_ObjFunc << std::endl;
//     if (config.cover_mode){
//         cout << "cover mode n1" << endl;
//         std::cout << "coverages_filename: " << config.coverages_filename << std::endl;
//         std::cout << "TypeSubarea: " << config.TypeSubarea << std::endl;
//     }
//     if (config.cover_mode_n2){
//         cout << "cover mode n2" << endl;
//         std::cout << "coverages_filename_n2: " << config.coverages_filename_n2 << std::endl;
//         std::cout << "TypeSubarea_n2: " << config.TypeSubarea_n2 << std::endl;
//     }
//     std::cout << "Method: " << config.Method << std::endl;
//     if (config.Method == "RSSV"){
//         std::cout << "Method_RSSV_sp: " << config.Method_RSSV_sp << std::endl;
//         std::cout << "Method_RSSV_fp: " << config.Method_RSSV_fp << std::endl;
//         std::cout << "size_subproblems_rssv: " << config.size_subproblems_rssv << std::endl;
//         std::cout << "MAX_ITE_SUBPROB_RSSV: " << config.MAX_ITE_SUBPROB_RSSV << std::endl;
//         std::cout << "CLOCK_LIMIT_SUBPROB_RSSV: " << config.CLOCK_LIMIT_SUBPROB_RSSV << std::endl;
//     }
//     if (config.Method_PostOpt != "null")
//         std::cout << "Method_PostOpt: " << config.Method_PostOpt << std::endl;
//     std::cout << "output_filename: " << config.output_filename << std::endl;
//     std::cout << "VERBOSE: " << config.VERBOSE << std::endl;
//     std::cout << "CLOCK_LIMIT: " << config.CLOCK_LIMIT << std::endl;
//     std::cout << "CLOCK_LIMIT_CPLEX: " << config.CLOCK_LIMIT_CPLEX << std::endl
//     std::cout << "BW_MULTIPLIER: " << config.BW_MULTIPLIER << std::endl;
//     if (config.fixed_threshold_distance > 0) std::cout << "fixed_threshold_distance: " << config.fixed_threshold_distance << std::endl;
//     std::cout << "add_threshold_distance_rssv: " << config.add_threshold_distance_rssv << std::endl;
//     std::cout << "add_generate_reports: " << config.add_generate_reports << std::endl;
//     std::cout << "add_break_callback: " << config.add_break_callback << std::endl;
//     std::cout << "threads_cnt: " << config.threads_cnt << std::endl;
//     std::cout << "seed: " << config.seed << std::endl;
//     cout << " -------------------------------------------------- " << endl;
//     cout << "\n\n";
    
// }