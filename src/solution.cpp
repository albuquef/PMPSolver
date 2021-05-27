#include <iomanip>
#include <cassert>
#include <utility>
#include "solution.hpp"

Solution::Solution(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations) {
    this->instance = std::move(instance);
    this->p_locations = std::move(p_locations);
//    fullEval();
    fullCapEval();
}

//void Solution::fullEval() {
//    objective = 0;
//    for (auto cust:instance->getCustomers()) {
//        auto loc = getClosestpLoc(cust);
//        auto dist = instance->getWeightedDist(loc, cust);
//        objective += dist;
//        assignment[cust] = my_pair{loc, dist};
////        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
//    }
//}

void Solution::fullCapEval() {
    objective = 0;
    // Determine unassigned customer's urgencies
    // Sort customers by decreasing urgencies
    // Assign customers, until some capacity is full
    // Recompute urgencies and repeat (for unassigned customers and open locations only)

}

uint_t Solution::getClosestpLoc(uint_t cust) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    dist_t dist;
    uint_t loc_closest;
    for (auto loc:p_locations) {
        dist = instance->getWeightedDist(loc, cust);
        if (dist <= dist_min) {
            dist_min = dist;
            loc_closest = loc;
        }
    }

    return loc_closest;
}

void Solution::print() {
    cout << "p locations: ";
    for (auto p:p_locations) {
        cout << p << " ";
    }
//    for (auto cust:instance->getCustomers()) {
//        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
//    }
    cout << endl;
    cout << setprecision(15) << "objective: " << objective << endl;
}

const unordered_set<uint_t> &Solution::get_pLocations() const {
    return this->p_locations;
}

//void Solution::replaceLocation(uint_t loc_old, uint_t loc_new) {
//    // Update p_locations
//    p_locations.erase(loc_old);
//    p_locations.insert(loc_new);
//    // Update assignment and objective
//    for (auto cust:instance->getCustomers()) {
//        auto dist_old = assignment[cust].dist;
//        auto dist_new = instance->getWeightedDist(loc_new, cust);
//        if (assignment[cust].node == loc_old) { // cust must be reassigned to some other p location
//            auto loc_closest = getClosestpLoc(cust);
//            auto dist_closest = instance->getWeightedDist(loc_closest, cust);
//            assignment[cust] = my_pair {loc_closest, dist_closest};
//            objective -= dist_old;
//            objective += dist_closest;
//        } else if (dist_old - dist_new > TOLERANCE) { // cust may be reassigned to loc_new
//            assignment[cust] = my_pair {loc_new, dist_new};
//            objective -= dist_old;
//            objective += dist_new;
//        }
////        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
//    }
//}

dist_t Solution::get_objective() {
    return objective;
}

//void Solution::printAssignment() {
//    for (auto p_loc:p_locations) {
//        cout << "location: " << p_loc << endl;
//        cout << "customers: ";
//        for (auto cust:instance->getCustomers()) {
//            if (assignment[cust].node == p_loc) {
//                cout << cust << " ";
//            }
//        }
//        cout << endl;
//        for (auto cust:instance->getCustomers()) {
//            if (assignment[cust].node == p_loc) {
//                printf("%i (%.2f) ", cust, assignment[cust].dist);
//            }
//        }
//        cout << endl;
//    }
//}

//void Solution::exportSolution(const string& output_filename) {
//    ofstream output_file(output_filename);
//    if (output_file.is_open()) {
//        output_file << "p_locations\n";
//        for (auto loc:p_locations) output_file << loc << endl;
//        output_file << endl;
//        output_file << "assignment\n";
//        for (auto loc:p_locations) {
//            output_file << loc << ": ";
//            for (auto cust:instance->getCustomers()) {
//                if (assignment[cust].node == loc) {
//                    output_file << cust << " ";
//                }
//            }
//            output_file << endl;
//        }
//        output_file << endl;
//        output_file << "distances\n";
//        for (auto loc:p_locations) {
//            output_file << loc << ": ";
//            for (auto cust:instance->getCustomers()) {
//                if (assignment[cust].node == loc) {
//                    output_file << cust << " (" << assignment[cust].dist << ") ";
//                }
//            }
//            output_file << endl;
//        }
//    } else {
//        cerr << "Error while opening output file\n";
//    }
//}






