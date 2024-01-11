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
#include "PMP.hpp"
#include "VNS.hpp"

using namespace std;

int main(int argc, char *argv[]) {

    // Required parameters
    uint_t p = 0;
    string dist_matrix_filename;
    string labeled_weights_filename;
    string capacities_filename;
    // Optional parameters
    int threads_cnt = (int) getAvailableThreads();
    int mode = 0;
    int seed = 1;
    string output_filename;
    int MAX_ITE_TB = 10;
    int MAX_ITE_VNS = 100;
    bool isBin_CPMP = false;
    // string typeProblem;
    // string typeHeuristic;
    string TypeService;


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
                configOverride.insert("time");

            } else if (strcmp(argv[i], "-o") == 0) {

                output_filename = argv[i + 1];
                configOverride.insert("output");

            } else if (strcmp(argv[i], "-c") == 0) {

                capacities_filename = argv[i + 1];
                configOverride.insert("capacities");

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
            }  else if (argv[i][0] == '?' || (strcmp(argv[i],"--help")==0)) {
            
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
                    "-t || --time\n"
                    "\tTakes the CPU time in second after which the program quits automatically.\n\n"
                    "--seed : \n"
                    "\tseed of the random generator (default = 1)\n\n"
                    "--mode\n"
                    "\tmode of operation\n\n"
                    "There are 12 mods :\n\n"

                    
                    "Mode 1 : \n"
                    "\tFirst mode use Exact Method for PMP\n\n"
                    "Mode 2 : \n"
                    "\tSecond mode use Exact Method for a continuos cPMP\n\n"
                    "Mode 3 : \n"
                    "\tThird mode use Exact Method for a binary cPMP\n\n"

                    "Mode 4 : \n"
                    "\tFourth mode use TB Heuristic with standard PMP\n\n"
                    "Mode 5 : \n"
                    "\tFifth mode use TB Heuristic with cPMP\n\n"
                    "Mode 6 : \n"
                    "\tSixth mode use full RSSV Heuristic with PMP\n\n"
                    "Mode 7 : \n"
                    "\tSeventh mode use full RSSV Heuristic with cPMP\n\n"
                    "Mode 8 : \n"
                    "\tEighth mode use TB Heuristic with a fixed percentage parameter and with standard PMP\n\n"
                    "Mode 9 : \n"
                    "\tNinth mode use TB Heuristic with a fixed percentage parameter and with cPMP\n\n"

                    "Mode 10 : \n"
                    "\tTenth mode use basic VNS Heuristic with PMP\n\n"
                    "Mode 11 : \n"
                    "\tTenth mode use basic VNS Heuristic with cPMP\n\n"
                    "Mode 11 : \n"
                    "\tTwelfth mode use basic RSSV (using VNS) Heuristic with cPMP\n\n"



                    "Generic example : \n"
                    "\t./large_PMP -p <number_of_medians> -dm <path_to_matrix_of_distance> -w <path_to_weigths_of_customer> -c <path_to_location_capacities> --mode <no_of_mode>\n\n"

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
    config.setFromConfig(&dist_matrix_filename, "distance_matrix");
    config.setFromConfig(&output_filename, "output");
    // config.setFromConfig(&output_vars_filename, "output_vars");
    // config.setFromConfig(&output_table_filename, "output_table");
    config.setFromConfig(&labeled_weights_filename, "weights");
    config.setFromConfig(&threads_cnt, "threads");
    config.setFromConfig(&mode, "mode");
    config.setFromConfig(&seed, "seed");
    config.setFromConfig(&CLOCK_LIMIT, "time");
    config.setFromConfig(&TOLERANCE_CPT, "toleranceCpt");
    config.setFromConfig(&K, "k");
    config.setFromConfig(&PERCENTAGE, "percentage");
    config.setFromConfig(&TypeService, "service");

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

    // Do something
    auto start = tick();
    switch (mode) {
        case 1: {
            cout << "-------------------------------------------------\n";
            cout << "Exact method PMP\n";
            cout << "-------------------------------------------------\n";
            PMP pmp(make_shared<Instance>(instance), "PMP");
            pmp.run();
            pmp.saveVars(output_filename,mode);
            pmp.saveResults(output_filename,mode);
            auto solution = pmp.getSolution_std();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 2: {
            cout << "-------------------------------------------------\n";
            cout << "Exact method cPMP continuos\n";
            cout << "-------------------------------------------------\n";
            PMP pmp(make_shared<Instance>(instance), "CPMP");
            pmp.run();
            pmp.saveVars(output_filename,mode);
            pmp.saveResults(output_filename,mode);
            auto solution = pmp.getSolution_cap();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 3: {
            cout << "-------------------------------------------------\n";
            cout << "Exact method cPMP binary\n";
            cout << "-------------------------------------------------\n";
            PMP pmp(make_shared<Instance>(instance), "CPMP", true);
            pmp.run();
            pmp.saveVars(output_filename,mode);
            pmp.saveResults(output_filename,mode);
            auto solution = pmp.getSolution_cap();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 4: {
            cout << "-------------------------------------------------\n";
            cout << "TB heuristic - standard PMP\n";
            cout << "-------------------------------------------------\n";
            TB heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.run(true,MAX_ITE_TB);
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 5: {
            cout << "-------------------------------------------------\n";
            cout << "TB heuristic - cPMP\n";
            cout << "-------------------------------------------------\n";
            TB heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.run_cap(true,MAX_ITE_TB);
            // solution.print();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 6: {
            // Extract filtered instance
            cout << "-------------------------------------------------\n";
            cout << "RSSV heuristic - standard PMP\n";
            cout << "-------------------------------------------------\n";
            RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
            CLOCK_THREADED = true;
            auto filtered_instance = metaheuristic.run(THREAD_NUMBER);
            // solve filtered instance by the TB heuristic
            TB heuristic(filtered_instance, seed);
            auto solution = heuristic.run(true,MAX_ITE_TB);
            // cout << "Final solution:\n";
            // solution.print();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 7: {
            // Extract filtered instance
            cout << "-------------------------------------------------\n";
            cout << "RSSV heuristic - cPMP\n";
            cout << "-------------------------------------------------\n";
            RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
            CLOCK_THREADED = true;

            // auto filtered_instance = metaheuristic.run_CAP(THREAD_NUMBER);
            auto filtered_instance = metaheuristic.run(THREAD_NUMBER);
            // solve filtered instance by the TB heuristic
            TB heuristic(filtered_instance, seed);
            auto solution = heuristic.run_cap(true,MAX_ITE_TB);
            cout << "Final solution:\n";
            solution.print();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 8: {
            cout << "-------------------------------------------------\n";
            cout << "TBPercentage heuristic - standard PMP\n";
            cout << "-------------------------------------------------\n";
            TBPercentage heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.run(true);
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 9: {
            cout << "-------------------------------------------------\n";
            cout << "TBPercentage heuristic - cPMP\n";
            cout << "-------------------------------------------------\n";
            TBPercentage heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.run_cap(true);
            solution.print();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 10: {
            cout << "-------------------------------------------------\n";
            cout << "VNS heuristic - PMP\n";
            cout << "-------------------------------------------------\n";
            VNS heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.runVNS_std(true,MAX_ITE_VNS);
            // solution.print();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        case 11: {
            cout << "-------------------------------------------------\n";
            cout << "VNS heuristic - cPMP\n";
            cout << "-------------------------------------------------\n";
            VNS heuristic(make_shared<Instance>(instance), seed);
            auto start_time = high_resolution_clock::now();
            auto solution = heuristic.runVNS_cap(output_filename,mode,false,MAX_ITE_VNS);
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            // solution.print();
            solution.saveAssignment(output_filename,mode);
            solution.saveResults(output_filename, mode, elapsed_time,0);
            break;
        }case 12: {
            // Extract filtered instance
            cout << "-------------------------------------------------\n";
            cout << "RSSV using VNS heuristic - cPMP\n";
            cout << "-------------------------------------------------\n";
            RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
            CLOCK_THREADED = true;

            // auto filtered_instance = metaheuristic.run_CAP(THREAD_NUMBER);
            auto filtered_instance = metaheuristic.run(THREAD_NUMBER);
            // solve filtered instance by the TB heuristic
            VNS heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.runVNS_cap(output_filename,mode,false,MAX_ITE_VNS);
            cout << "Final solution:\n";
            solution.print();
            solution.saveAssignment(output_filename,mode);
            break;
        }
        default: {
            cout << "Experimental branch\n";
            TB heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.initHighestCapSolution();
            solution.print();
        }
    }

    cout << endl;
    tock(start);

    return 0;
}