#include <set>
#include <cstring>
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    // Required parameters
    uint_t p = 0;
    string dist_matrix_filename;
    string labeled_weights_filename;
    string capacities_filename;
    // Optional parameters
    int threads_cnt = 4;
    int mode = 0;
    int seed = 1;
    string output_filename;

    // Users want help with "?"
    if((strcmp(argv[1],"?")==0) || (strcmp(argv[1],"help")==0)) {
        
    }

    // Parameters parsing
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' || argv[i][0] == '?') {
            if (strcmp(argv[i], "-p") == 0) {
                p = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-dm") == 0) {
                dist_matrix_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-w") == 0) {
                labeled_weights_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-th") == 0) {
                threads_cnt = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "--mode") == 0) {
                mode = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "--seed") == 0) {
                seed = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-o") == 0) {
                output_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-c") == 0) {
                capacities_filename = argv[i + 1];
            } else if (argv[i][0] == '?' || (strcmp(argv[i],"--help")==0)) {
                cout << 
                    "\nTo run the program, you have to specify some options : \n\n"

                    "-p : \n" 
                    "the number of medians to select\n\n"
                    "-dm : \n"
                    "path to the distance matrix file\n\n"
                    "-w : \n"
                    "path to the file with customer weights (=no. of people living at a idividual customer locations)\n\n"
                    "-c : \n"
                    "path to the file with location capacities (needed even for solving standard PMP)\n\n"

                    "\nMoreover, you have some optional options : \n\n"
                    
                    "-o : \n"
                    "path to an output file, for ewporting a solution\n\n"
                    "-th : \n"
                    "no. of subproblems solved by the RSSV in parallel (default = 4)\n\n"
                    "--seed : \n"
                    "seed of the random generator (default = 1)\n\n"
                    "--mode\n"
                    "mode of operation\n\n"
                    "There are 4 mods :\n\n"

                    "Mode 1 : \n"
                    "First mode use TB Heuristic with standard PMP\n\n"
                    "Mode 2 : \n"
                    "Second mode use TB Heuristic with cPMP\n\n"
                    "Mode 3 : \n"
                    "Third mode use full RSSV Heuristic with PMP\n\n"
                    "Mode 4 : \n"
                    "Fourth mode use full RSSV Heuristic with cPMP\n\n"

                    "Generic example : \n"
                    "./large_PMP -p <number_of_medians> -dm <path_to_matrix_of_distance> -w <path_to_weigths_of_customer> -c <path_to_location_capacities> --mode <no_of_mode>\n\n"

                    "Usage example with Toulon from build directory\n"
                    "./large_PMP -p 5 -dm ../data/toulon/dist_matrix.txt -w ../data/toulon/cust_weights.txt -c ../data/toulon/loc_capacities --mode 3 -o output.txt\n"
                    "It solves the standard PMP with 5 medians for the Toulon instance. Solution is exported to file 'output.txt'"
                << endl;
                exit(0); 
            } else {
                cerr << "Unknown parameter: " << argv[i] << endl;
                exit(1);
            }
        }
    }

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

    // Load instance
    Instance instance(dist_matrix_filename, labeled_weights_filename, capacities_filename, p, ' ');
//    omp_set_num_threads(1);

    // Do something
    auto start = tick();
    switch (mode) {
        case 1: {
            cout << "TB heuristic - standard PMP\n";
            TB heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.run(true);
            solution.printAssignment(output_filename);
            break;
        }
        case 2: {
            cout << "TB heuristic - cPMP\n";
            TB heuristic(make_shared<Instance>(instance), seed);
            auto solution = heuristic.run_cap(true);
            solution.print();
            solution.printAssignment(output_filename);
            break;
        }
        case 3: {
            // Extract filtered instance
            cout << "RSSV heuristic - standard PMP\n";
            RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
            auto filtered_instance = metaheuristic.run(threads_cnt);
            // solve filtered instance by the TB heuristic
            TB heuristic(filtered_instance, 1);
            auto solution = heuristic.run(true);
            cout << "Final solution:\n";
            solution.print();
            solution.printAssignment(output_filename);
            break;
        }
        case 4: {
            // Extract filtered instance
            cout << "RSSV heuristic - cPMP\n";
            RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
            auto filtered_instance = metaheuristic.run(threads_cnt);
            // solve filtered instance by the TB heuristic
            TB heuristic(filtered_instance, 1);
            auto solution = heuristic.run_cap(true);
            cout << "Final solution:\n";
            solution.print();
            solution.printAssignment(output_filename);
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