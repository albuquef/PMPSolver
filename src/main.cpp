#include <set>
#include <iostream>
#include <unistd.h>
#include "instance.hpp"
#include "solution.hpp"
#include "TB.hpp"
#include "RSSV.hpp"

using namespace std;

int main() {
    string loc_filename = "./provided/toulon/toulon_destination.txt"; // p = 5
    string cust_filename = "./provided/toulon/toulon_origin.txt";
    string dist_filename = "./provided/toulon/toulon_orig_x_tps.txt";

//    string loc_filename = "./provided/paca/global_destination.txt"; // p = 100
//    string cust_filename = "./provided/paca/global_origin.txt";
//    string dist_filename = "./provided/paca/global_orig_x_tps.txt";

    Instance instance(loc_filename, cust_filename, dist_filename, 5);

//    auto start = tick();
//    default_random_engine generator;
//    Instance instance1 = instance.sampleSubproblem(800, 800, 4, &generator);
//    tock(start);

//    cout << instance.getDist(690, 1) << endl; // Should return 0.226392658 for Toulon
//    cout << instance1.getDist(690, 1) << endl; // Should return 0.226392658 for Toulon

    instance.print();
//    instance1.print();

//    unordered_set<uint_t> locations = {534, 529, 298, 580, 355}; // Toulon optimal solution, objective: 1112707.98040259
//    unordered_set<uint_t> locations = {37743, 32521, 53446, 9467, 1}; // PACA, locations with missing distances
//    Solution solution(&instance, locations);
//    solution.print();

//    auto start = tick();
//
//    TB heuristic(&instance, 1);
//    auto sol = heuristic.run();
//    sol.print();
//
//    cout << "TB: ";
//    tock(start);

    RSSV metaheuristic(&instance, 1, 763);
//    metaheuristic.run();

    return 0;
}
