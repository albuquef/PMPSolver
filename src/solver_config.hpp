#ifndef PMPSOLVER_CONFIG_HPP
#define PMPSOLVER_CONFIG_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <set>
#include <stdexcept>

#include "globals.hpp"
#include "config_parser.cpp"
#include "utils.hpp"


using namespace std;

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
    uint_t threads_cnt = static_cast<uint_t>(getAvailableThreads());  // Default initialized based on available threads
    int mode = 0;
    int seed = 1;
    string output_filename="output.txt";
    string Method="TB_PMP";
    string Method_RSSV_sp="TB_PMP";
    string Method_RSSV_fp="TB_PMP";
    string Method_PostOpt;
    bool VERBOSE = false;
    uint_t TOLERANCE_CPT = 10;
    uint_t K = 1;
    uint_t PERCENTAGE = 0;
    double CLOCK_LIMIT = 0;
    double CLOCK_LIMIT_CPLEX = 0;
    double CLOCK_LIMIT_SUBPROB_RSSV = 0;
    uint_t MAX_ITE_SUBPROB_RSSV = 0;
    uint_t size_subproblems_rssv = 800;
    uint_t size_final_prob_rssv = 800;

    double BW_MULTIPLIER = 1.0;
    double fixed_threshold_distance = 0.0;
    bool add_threshold_distance_rssv = false;
    bool add_generate_reports = false;
    bool add_break_callback = false;

    bool cover_mode = false;
    bool cover_mode_n2 = false;
    uint_t cust_max_id = 0;
    uint_t loc_max_id = 0;
    bool IsWeighted_ObjFunc = false;
    // std::set<const char*> configOverride;
    std::set<std::string> configOverride;
    std::string configPath = "config.toml";
};

void parseArguments(int argc, char* argv[], Config& config);

void setupConfig(Config& config, std::set<std::string>& configOverride);

void checkRequiredParameters(const Config& config);

void setBoolConfigOption(bool &configOption, const char* value, const std::string &paramName, std::set<std::string> &configOverride);
// void setMethod_Name(string method);

// void printINFOparameters(const Config& config)

#endif // PMPSOLVER_CONFIG_HPP