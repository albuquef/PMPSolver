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

    // Parameters parsing
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-p") == 0) {
                p = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-dm") == 0) {
                dist_matrix_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-w") == 0) {
                labeled_weights_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-th") == 0) {
                threads_cnt = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-mode") == 0) {
                mode = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-seed") == 0) {
                seed = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-o") == 0) {
                output_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-c") == 0) {
                capacities_filename = argv[i + 1];
            } else {
                cerr << "Unknown parameter: " << argv[i] << endl;
                exit(1);
            }
        }
    }

    // Parameters check
    if (p == 0) {
        cerr << "No. of medians -p not given.\n";
        exit(1);
    } else if (dist_matrix_filename.empty()) {
        cerr << "Distance matrix -dm not given.\n";
        exit(1);
    } else if (labeled_weights_filename.empty()) {
        cerr << "Customer weights -w not given.\n";
        exit(1);
    } else if (capacities_filename.empty()) {
        cerr << "Location capacities -c not given.\n";
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
