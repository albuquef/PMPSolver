#include <set>
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"

using namespace std;

int main() {
    // Toulon files
    // Old parser
    //    string loc_filename = "./provided/toulon/locations.txt"; // p = 5
    //    string cust_filename = "./provided/toulon/customers.txt";
    //    string dist_filename = "./provided/toulon/distances.txt";
    //    string weights_filename = "./provided/toulon/weights.txt";
    //    Instance instance(loc_filename, cust_filename, dist_filename, weights_filename, 5);
    // New parser with my files
        string labeled_weights_filename = "./provided/toulon/weights_labeled.txt";
        string dist_matrix_filename = "./provided/toulon/dist_matrix.txt";
        Instance instance(dist_matrix_filename, labeled_weights_filename, 5, ' ');
    // New parser with provided files
    //    string labeled_weights_filename = "./provided/toulon/pts_origines_toulon.csv";
    //    string dist_matrix_filename = "./provided/toulon/matrice_dist_toulon.csv";
    //    Instance instance(dist_matrix_filename, labeled_weights_filename, 5, ';');

    // PACA files
    // Old parser
    //    string loc_filename = "./provided/paca/locations.txt"; // p = 100
    //    string cust_filename = "./provided/paca/customers.txt";
    //    string dist_filename = "./provided/paca/distances.txt";
    //    string weights_filename = "./provided/paca/weights.txt";
    //    Instance instance(loc_filename, cust_filename, dist_filename, weights_filename, 100);
    // New PACA files
    //        string labeled_weights_filename = "./provided/matdistpaca_600_1500_tps_90/weights_labeled.txt";
    //        string dist_matrix_filename = "./provided/matdistpaca_600_1500_tps_90/dist_matrix.txt";
    //        Instance instance(dist_matrix_filename, labeled_weights_filename, 100, ' ');

    //    Simple tests
    //    default_random_engine generator;
    //    Instance instance1 = instance.sampleSubproblem(800, 800, 100, &generator);
    //    cout << instance.getWeightedDist(690, 1) << endl; // Should return 0.226392658 for Toulon
    //    cout << instance1.getWeightedDist(690, 1) << endl; // Should return 0.226392658 for Toulon
    //    unordered_set<uint_t> locations = {534, 529, 298, 580, 355}; // Toulon optimal solution, objective: 1112707.98040259

//    auto start = tick();
//    TB heuristic(&instance, 2);
//    auto sol = heuristic.run(true);
//    sol.print();
//    tock(start);
//    sol.printAssignment();

//  RSSV heuristic
    auto start = tick();
    RSSV metaheuristic(&instance, 1, 800);
    auto sol = metaheuristic.run(4);
    cout << "RSSV: ";
    tock(start);

    return 0;
}
