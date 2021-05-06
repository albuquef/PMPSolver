#include <set>
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"

using namespace std;

int main() {
    string loc_filename = "./provided/toulon/toulon_destination.txt"; // p = 5
    string cust_filename = "./provided/toulon/toulon_origin.txt";
    string dist_filename = "./provided/toulon/toulon_tps.txt";
    string weights_filename = "./provided/toulon/pts_origines_toulon.txt";

//    string loc_filename = "./provided/paca/global_destination.txt"; // p = 100
//    string cust_filename = "./provided/paca/global_origin.txt";
//    string dist_filename = "./provided/paca/global_tps.txt";
//    string weights_filename = "./provided/paca/global_pts_origines.txt";

    Instance instance(loc_filename, cust_filename, dist_filename, weights_filename, 5);

//    Simple tests
//    default_random_engine generator;
//    Instance instance1 = instance.sampleSubproblem(800, 800, 4, &generator);
//    cout << instance.getWeightedDist(690, 1) << endl; // Should return 0.226392658 for Toulon
//    cout << instance1.getWeightedDist(690, 1) << endl; // Should return 0.226392658 for Toulon
//    unordered_set<uint_t> locations = {534, 529, 298, 580, 355}; // Toulon optimal solution, objective: 1112707.98040259

//    TB heuristic
    auto start = tick();
    TB heuristic(&instance, 2);
    auto sol = heuristic.run(true);
    sol.print();
    tock(start);

    sol.printAssignment();

//  RSSV heuristic
//    auto start = tick();
//    RSSV metaheuristic(&instance, 1, 800);
//    metaheuristic.run(1);
//    tock(start);

//    auto sol = heuristic.initRandomSolution();
//    sol.print(); // 1727438.83154983
//    sol.replaceLocation(1, 2);
//    sol.print(); // 1715925.48594329
    return 0;
}
