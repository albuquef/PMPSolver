#include <set>
#include <cstring>
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    // Obligatory parameters
    uint_t p = 0;
    string dist_matrix_filename;
    string labeled_weights_filename;
    // Optional parameters
    int threads_cnt = 4;
    int mode = 0;
    int seed = 1;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-p") == 0) {
                p = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-dm") == 0) {
                dist_matrix_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-w") == 0) {
                labeled_weights_filename = argv[i + 1];
            } else if (strcmp(argv[i], "-th") == 0) {
                threads_cnt = 4;
            } else if (strcmp(argv[i], "-mode") == 0) {
                mode = stoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-seed") == 0) {
                seed = stoi(argv[i + 1]);
            } else {
                cerr << "Unknown parameter: " << argv[i] << endl;
                exit(1);
            }
        }
    }

    if (p == 0) {
        cerr << "No. of medians -p not given.\n";
        exit(1);
    } else if (dist_matrix_filename.empty()) {
        cerr << "Distance matrix -dm not given.\n";
        exit(1);
    } else if (labeled_weights_filename.empty()) {
        cerr << "Customer weights -w not given.\n";
        exit(1);
    }

    Instance instance(dist_matrix_filename, labeled_weights_filename, p, ' ');

    auto start = tick();
    switch (mode) {
        case 1:
            cout << "Experimental branch\n";
            break;
        case 2: { // TB heuristic
            TB heuristic(make_shared<Instance>(instance), seed);
            heuristic.run(true);
            cout << "TB: ";
            break;
        }
        default: // RSSV heuristic
            RSSV metaheuristic(make_shared<Instance>(instance), seed, 800);
            auto sol = metaheuristic.run(threads_cnt);
            cout << "RSSV: ";
    }
    tock(start);

    return 0;
}
