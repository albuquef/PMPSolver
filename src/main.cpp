#include <set>
#include <cstring>
#include <string>
#include <chrono> // for time-related functions
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

int seed = 1;
Solution_MAP solution_map;
Solution_std methods_PMP(const shared_ptr<Instance>& instance, const string typeMethod, const string& output_filename);
Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const string typeMethod, const string& output_filename);

using namespace std;

int main(int argc, char *argv[]) {

    // Required parameters
    uint_t p = 0;
    string dist_matrix_filename;
    string labeled_weights_filename;
    string capacities_filename;
    string TypeService;
    string coverages_filename;
    string TypeSubarea;
    // Optional parameters
    uint_t threads_cnt = (uint_t) getAvailableThreads();
    int mode = 0;
    int seed = 1;
    string output_filename;
    string Method;
    string Method_RSSV_sp;
    string Method_RSSV_fp;
    bool cover_mode = false;


    // default config path
    std::string configPath = "config.toml";
    std::set<const char*> configOverride;


    // Parameters parsing
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' || argv[i][0] == '?') {
            if (strcmp(argv[i], "-p") == 0) {

                p = stoi(argv[i + 1]);
                configOverride.insert("p");

            } else if (strcmp(argv[i], "-v") == 0 ||
                       strcmp(argv[i], "-verbose") == 0) {

                VERBOSE = true;
                configOverride.insert("verbose");

            } else if (strcmp(argv[i], "-config") == 0) {
                configPath = argv[i + 1];
            } else if (strcmp(argv[i], "-dm") == 0) {

                dist_matrix_filename = argv[i + 1];
                configOverride.insert("distance_matrix");

            } else if (strcmp(argv[i], "-w") == 0) {

                labeled_weights_filename = argv[i + 1];
                configOverride.insert("weights");

            } else if (strcmp(argv[i], "-th") == 0) {

                threads_cnt = stoi(argv[i + 1]);
                configOverride.insert("threads");

            } else if (strcmp(argv[i], "--mode") == 0) {

                mode = stoi(argv[i + 1]);
                configOverride.insert("mode");

            } else if (strcmp(argv[i], "--seed") == 0) {

                seed = stoi(argv[i + 1]);
                configOverride.insert("seed");

            } else if (strcmp(argv[i], "-t") == 0 || 
                       strcmp(argv[i], "-time") == 0) {

                setClockLimit(stoi(argv[i + 1]));
                CLOCK_LIMIT = stod(argv[i + 1]);
                configOverride.insert("time");

            } else if (strcmp(argv[i], "-time_cplex") == 0) {

                CLOCK_LIMIT_CPLEX = stod(argv[i + 1]);
                configOverride.insert("time_cplex");

            } else if (strcmp(argv[i], "-o") == 0) {

                output_filename = argv[i + 1];
                configOverride.insert("output");

            } else if (strcmp(argv[i], "-c") == 0) {

                capacities_filename = argv[i + 1];
                configOverride.insert("capacities");

            } else if (strcmp(argv[i], "-cover") == 0) {

                coverages_filename = argv[i + 1];
                configOverride.insert("coverages");

            } else if (strcmp(argv[i], "-toleranceCpt") == 0){
                
                TOLERANCE_CPT = stoi(argv[i + 1]);
                configOverride.insert("toleranceCpt");
                
            } else if (strcmp(argv[i], "-k") == 0){
                
                K = stoi(argv[i + 1]);
                configOverride.insert("k");
                
            } else if (strcmp(argv[i], "-percentage") == 0){
                
                PERCENTAGE = stoi(argv[i + 1]);
                configOverride.insert("percentage");
            } else if (strcmp(argv[i], "-service") == 0){
                
                TypeService = argv[i + 1];
                configOverride.insert("service");                
            } else if (strcmp(argv[i], "-subarea") == 0){
                
                TypeSubarea = argv[i + 1];
                configOverride.insert("subarea");                
            } else if (strcmp(argv[i], "-method") == 0){
                
                Method = argv[i + 1];
                configOverride.insert("method");                
            } else if (strcmp(argv[i], "-method_rssv_sp") == 0){
                
                Method_RSSV_sp = argv[i + 1];
                configOverride.insert("method_rssv_sp");                
            } else if (strcmp(argv[i], "-method_rssv_fp") == 0){
                
                Method_RSSV_fp = argv[i + 1];
                configOverride.insert("method_rssv_fp");                
            } else if (strcmp(argv[i], "-cover_mode") == 0){
                if (strcmp(argv[i + 1], "true") == 0 || strcmp(argv[i + 1], "1") == 0){
                    cover_mode = true;
                } else if (strcmp(argv[i + 1], "false") == 0 || strcmp(argv[i + 1], "0") == 0){
                    cover_mode = false;
                } else {
                    cerr << "Unknown parameter [cover mode]: " << argv[i] << endl;
                    // exit(1);
                }
                // cover_mode = argv[i + 1];
                configOverride.insert("cover_mode");                
            } else if (argv[i][0] == '?' || (strcmp(argv[i],"--help")==0)) {
            
                cout << 
                    "\nTo run the program, you have to specify some options : \n\n"

                    "-p : \n" 
                    "\tthe number of medians to select\n\n"
                    "-dm : \n"
                    "\tpath to the distance matrix file\n\n"
                    "-w : \n"
                    "\tpath to the file with customer weights (=no. of people living at a idividual customer locations)\n\n"
                    "-c : \n"
                    "\tpath to the file with location capacities (needed even for solving standard PMP)\n\n"

                    "\nMoreover, you have some optional options : \n\n"
                    
                    "-o : \n"
                    "\tpath to an output file, for ewporting a solution\n\n"
                    "-th : \n"
                    "\tno. of subproblems solved by the RSSV in parallel (default = 4)\n\n"
                    "-t || -time\n"
                    "\tTakes the CPU time in second after which the program quits automatically.\n\n"
                    "-time_cplex\n"
                    "\t Time limit applied to cplex, if is equal to 1 it means no time limit.\n\n"
                    "--seed : \n"
                    "\tseed of the random generator (default = 1)\n\n"
                    // "--mode\n"
                    // "\tmode of operation\n\n"
                    // "There are 12 mods :\n\n"

                    "-Method : \n"
                    "\tMethod to solve the problem, there are 8 methods :\n"
                    "\t EXACT_PMP, EXACT_CPMP, EXACT_CPMP_BIN : exact methdos using cplex\n"
                    "\t TB_PMP, TB_CPMP : Teitz and Bart heuristic \n"
                    "\t VNS_PMP, VNS_CPMP : Variable Neighbourhood Search heuristic \n\n"
                    "\t RSSV : Random Search and Spatial Voting algorithm”\n\n"

                    "-method_rssv_sp : \n"
                    "\tMethod to solve the subproblems of the RSSV, there are 7 methods :\n"
                    "\t Same methods as 'Method' without RSSV \n"

                    "-method_rssv_fp : \n"
                    "\tMethod to solve the final problem of the RSSV, there are 7 methods :\n"
                    "\t Same methods as 'Method' without RSSV \n"

                    "Generic example : \n"
                    "\t./large_PMP -p <number_of_medians> -dm <path_to_matrix_of_distance> -w <path_to_weigths_of_customer> -c <path_to_location_capacities> -method <type_of_method>\n\n"

                    "Usage example with Toulon from build directory\n"
                    "\t./large_PMP -p 5 -dm ../data/toulon/dist_matrix.txt -w ../data/toulon/cust_weights.txt -c ../data/toulon/loc_capacities --mode 3 -o output.txt\n"
                    "It solves the standard PMP with 5 medians for the Toulon instance. Solution is exported to file 'output.txt'"
                << endl;
                exit(0); 
            } else {
                cerr << "Unknown parameter: " << argv[i] << endl;
                exit(1);
            }
        }
    }

        // Debugging: Print the contents of configOverride
    std::cout << "Debug: configOverride contents: ";
    for (const auto& key : configOverride) {
        std::cout << key << " ";
    }
    std::cout << std::endl;


    // setup config
    ConfigParser config(configPath, configOverride);
    config.setFromConfig(&VERBOSE, "verbose");
    config.setFromConfig(&p, "p");
    config.setFromConfig(&capacities_filename, "capacities");
    config.setFromConfig(&coverages_filename, "coverages");
    config.setFromConfig(&dist_matrix_filename, "distance_matrix");
    config.setFromConfig(&output_filename, "output");
    config.setFromConfig(&labeled_weights_filename, "weights");
    config.setFromConfig(&threads_cnt, "threads");
    config.setFromConfig(&mode, "mode");
    config.setFromConfig(&seed, "seed");
    config.setFromConfig(&CLOCK_LIMIT, "time");
    config.setFromConfig(&CLOCK_LIMIT_CPLEX, "time_cplex");
    config.setFromConfig(&TOLERANCE_CPT, "toleranceCpt");
    config.setFromConfig(&K, "k");
    config.setFromConfig(&PERCENTAGE, "percentage");
    config.setFromConfig(&TypeService, "service");
    config.setFromConfig(&Method, "method");
    config.setFromConfig(&Method_RSSV_sp, "method_rssv_sp");
    config.setFromConfig(&Method_RSSV_fp, "method_rssv_fp");


    setThreadNumber(threads_cnt);

    // Parameters check
    if (p == 0) {
        cerr << "No. of medians -p not given.\n";
        cerr << "If you need help to use, add --help or a '?' after name of program.\n" ;
        exit(1);
    } else if (dist_matrix_filename.empty()) {
        cerr << "Distance matrix -dm not given.\n";
        cerr << "If you need help to use, add --help or a '?' after name of program.\n" ;
        exit(1);
    } else if (labeled_weights_filename.empty()) {
        cerr << "Customer weights -w not given.\n";
        cerr << "If you need help to use, add --help or a '?' after name of program.\n" ;
        exit(1);
    } else if (capacities_filename.empty()) {
        cerr << "Location capacities -c not given.\n";
        cerr << "If you need help to use, add --help or a '?' after name of program.\n" ;
        exit(1);
    }
    cout << "Loading instance...\n";
    // Load instance
    Instance instance(dist_matrix_filename, labeled_weights_filename, capacities_filename, p, ' ',TypeService);
//    omp_set_num_threads(1);

    if(!coverages_filename.empty() && cover_mode){
        // cover_mode = true;
        instance.ReadCoverages(coverages_filename,TypeSubarea, ' ');
    }

    cout << "[INFO] Instance loaded\n";
    instance.print();


    auto start = tick();
    cout << "-------------------------------------------------\n";
    if(Method == "EXACT_PMP" || Method == "TB_PMP" || Method == "VNS_PMP"){
        
        auto start_time = high_resolution_clock::now();
        Solution_std solution = methods_PMP(make_shared<Instance>(instance), Method, output_filename);
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            
        cout << "\nFinal solution:\n";
        solution.print();
        cout << "Final total elapsed time: " << elapsed_time << "s\n";
        solution.saveAssignment(output_filename,Method);
        solution.saveResults(output_filename, elapsed_time,0,Method);   
    } else if(Method == "EXACT_CPMP" || Method == "EXACT_CPMP_BIN" || Method == "TB_CPMP" || Method == "VNS_CPMP"){
        
        
        auto start_time = high_resolution_clock::now();
        Solution_cap solution = methods_CPMP(make_shared<Instance>(instance), Method, output_filename);
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();

        cout << "\nFinal solution:\n";
        solution.print();
        cout << "Final total elapsed time: " << elapsed_time << "s\n";  
        solution.saveAssignment(output_filename,Method);
        solution.saveResults(output_filename, elapsed_time,0,Method); 
        
    } else if(Method == "RSSV"){
        cout << "RSSV heuristic \n";
        cout << "-------------------------------------------------\n";
        RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
        CLOCK_THREADED = true;
        auto start_time_total = high_resolution_clock::now();
        


        shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance);
        if(Method_RSSV_sp == "EXACT_PMP" || Method_RSSV_sp == "TB_PMP" || Method_RSSV_sp == "VNS_PMP"){
            auto filtered_instance = metaheuristic.run(THREAD_NUMBER,Method_RSSV_sp);
        } else if(Method_RSSV_sp == "EXACT_CPMP" || Method_RSSV_sp == "EXACT_CPMP_BIN" || Method_RSSV_sp == "TB_CPMP" || Method_RSSV_sp == "VNS_CPMP"){
            auto filtered_instance = metaheuristic.run_CAP(THREAD_NUMBER,Method_RSSV_sp);
        }

        cout << "-------------------------------------------------\n";
        cout << "Final Problem RSSV heuristic \n";
        cout << "-------------------------------------------------\n";
        if(Method_RSSV_fp == "EXACT_PMP" || Method_RSSV_fp == "TB_PMP" || Method_RSSV_fp == "VNS_PMP"){
            
            auto start_time = high_resolution_clock::now();
            Solution_std solution = methods_PMP(filtered_instance, Method_RSSV_fp, output_filename);
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();

            cout << "\nFinal solution:\n";
            solution.print();
            cout << "Final problem elapsed time: " << elapsed_time << "s\n";
            cout << "Final total elapsed time: " << duration_cast<seconds>(current_time - start_time_total).count() << "s\n";
            solution.saveAssignment(output_filename,Method);
            solution.saveResults(output_filename, elapsed_time,0,Method); 
        } else if(Method_RSSV_fp == "EXACT_CPMP" || Method_RSSV_fp == "EXACT_CPMP_BIN" || Method_RSSV_fp == "TB_CPMP" || Method_RSSV_fp == "VNS_CPMP"){
            auto start_time = high_resolution_clock::now();
            Solution_cap solution = methods_CPMP(filtered_instance, "RSSV_" + Method_RSSV_fp, output_filename);
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            if (Method_RSSV_fp == "EXACT_CPMP") solution.objEval();
            cout << "\nFinal solution:\n";
            solution.print();
            cout << "Final elapsed time: " << elapsed_time << "s\n";
            cout << "Final total elapsed time: " << duration_cast<seconds>(current_time - start_time_total).count() << "s\n";
            solution.saveAssignment(output_filename,"RSSV_" + Method_RSSV_fp);
            solution.saveResults(output_filename, elapsed_time,0,Method, Method_RSSV_sp, Method_RSSV_fp);
        }



    }
    cout << endl;
    tock(start);

    return 0;
}


Solution_std methods_PMP(const shared_ptr<Instance>& instance,const string typeMethod, const string& output_filename){


    Solution_std solution;
    cout << "-------------------------------------------------\n";
    // cout << "Method: " << typeMethod << endl;
    if (typeMethod == "EXACT_PMP"){
        cout << "Exact method PMP\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "PMP");
        pmp.run();
        pmp.saveVars(output_filename,typeMethod);
        pmp.saveResults(output_filename,typeMethod);
        solution = pmp.getSolution_std();
    }else if (typeMethod == "TB_PMP"){
        cout << "TB heuristic - standard PMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, seed);
        solution = heuristic.run(true,UB_MAX_ITER);
    }else if (typeMethod == "VNS_PMP"){
        cout << "VNS heuristic - PMP\n";
        cout << "-------------------------------------------------\n";
        VNS heuristic(instance, seed);
        solution = heuristic.runVNS_std(true,UB_MAX_ITER);
    }else{
        cout << "[ERROR] Method not found" << endl;
        exit(1);
    }
    return solution;
}
Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, string typeMethod, const string& output_filename){

    Solution_cap solution;
    Solution_MAP solution_map(instance);
    cout << "-------------------------------------------------\n";
    if (typeMethod == "EXACT_CPMP" || typeMethod == "RSSV_EXACT_CPMP"){
        cout << "Exact method cPMP continuos\n";
        cout << "-------------------------------------------------\n";    
        PMP pmp(instance, "CPMP");
        pmp.setGenerateReports(true);
        pmp.setCoverModel(true);
        // cout << "cover model: " << pmp.CoverModel << "\n";
        pmp.run();
        pmp.saveVars(output_filename,typeMethod);
        pmp.saveResults(output_filename,typeMethod);
        solution = pmp.getSolution_cap();
    }else if (typeMethod == "EXACT_CPMP_BIN" || typeMethod == "RSSV_EXACT_CPMP_BIN"){
        cout << "Exact method cPMP binary\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "CPMP", true);
        pmp.setGenerateReports(true);
        pmp.setCoverModel(true);
        pmp.run();
        pmp.saveVars(output_filename,typeMethod);
        pmp.saveResults(output_filename,typeMethod);
        solution = pmp.getSolution_cap();
    }else if (typeMethod == "TB_CPMP" || typeMethod == "RSSV_TB_CPMP"){
        cout << "TB heuristic - cPMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, seed);
        heuristic.setSolutionMap(solution_map);
        heuristic.setGenerateReports(true);
        heuristic.setMethod(typeMethod);
        solution = heuristic.run_cap(true,UB_MAX_ITER);
    }else if (typeMethod == "VNS_CPMP" || typeMethod == "RSSV_VNS_CPMP"){
        cout << "VNS heuristic - cPMP\n";
        cout << "-------------------------------------------------\n";
        VNS heuristic(instance, seed);
        heuristic.setSolutionMap(solution_map);
        heuristic.setGenerateReports(true);
        heuristic.setMethod(typeMethod);
        solution = heuristic.runVNS_cap(typeMethod,true,UB_MAX_ITER);
    
        if (solution.isSolutionFeasible()){
            cout << "Solution feasible\n";   
        } else {
            cout << "Solution not feasible\n";
        }
        

    }else{
        cout << "[ERROR] Method not found" << endl;
        exit(1);
    }
    return solution;
}