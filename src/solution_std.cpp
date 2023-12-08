#include <iomanip>
#include <utility>
#include "solution_std.hpp"

Solution_std::Solution_std(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations) {
    
    this->instance = std::move(instance);
    this->p_locations = std::move(p_locations);

    cout << "p locations: "; 
    for (auto loc:this->p_locations)
        cout << loc << " ";

    naiveEval();
}

void Solution_std::naiveEval() {
//    assert(p_locations.size() == instance->get_p());
    objective = 0;
    for (auto cust:instance->getCustomers()) {
        auto loc = getClosestpLoc(cust);
        auto dist = instance->getWeightedDist(loc, cust);
        objective += dist;
        assignment[cust] = my_pair{loc, dist};
//        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
    }
}

uint_t Solution_std::getClosestpLoc(uint_t cust) {
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

void Solution_std::print() {
    if (!VERBOSE) return;
    
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

const unordered_set<uint_t> &Solution_std::get_pLocations() const {
    return this->p_locations;
}

const vector<uint_t> &Solution_std::get_Locations() const {
    return this->instance->getLocations();
}

void Solution_std::replaceLocation(uint_t loc_old, uint_t loc_new) {
    // Update p_locations
    p_locations.erase(loc_old);
    p_locations.insert(loc_new);
    // Update assignment and objective
    for (auto cust:instance->getCustomers()) {
        auto dist_old = assignment[cust].dist;
        auto dist_new = instance->getWeightedDist(loc_new, cust);
        if (assignment[cust].node == loc_old) { // cust must be reassigned to some other p location
            auto loc_closest = getClosestpLoc(cust);
            auto dist_closest = instance->getWeightedDist(loc_closest, cust);
            assignment[cust] = my_pair {loc_closest, dist_closest};
            objective -= dist_old;
            objective += dist_closest;
        } else if (dist_old - dist_new > TOLERANCE) { // cust may be reassigned to loc_new
            assignment[cust] = my_pair {loc_new, dist_new};
            objective -= dist_old;
            objective += dist_new;
        }
//        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
    }
}

dist_t Solution_std::get_objective() {
    return objective;
}

void Solution_std::saveAssignment(const string& output_filename, int mode) {
    fstream file;
    streambuf *stream_buffer_cout = cout.rdbuf();
    string output_filename_final = output_filename + 
        "_p_" + to_string(p_locations.size()) + 
        "_mode_" + to_string(mode) +
        ".txt";

    // Open file if output_filename is not empty
    if (!output_filename_final.empty()) {
        file.open(output_filename_final, ios::out);
        streambuf *stream_buffer_file = file.rdbuf();
        cout.rdbuf(stream_buffer_file); // redirect cout to file
    }

    cout << setprecision(5) << "OBJECTIVE\n" << objective << endl << endl;

    cout << "P LOCATIONS\n";
    for (auto p_loc:p_locations) cout << p_loc << endl;
    cout << endl;

    cout << "LOCATION ASSIGNMENTS\nlocation: customers\n";
    for (auto loc:p_locations) {
        cout << loc << ": ";
        for (auto cust:instance->getCustomers()) {
            if (assignment[cust].node == loc) {
                cout << cust << " ";
            }
        }
        cout << endl;
    }
    cout << endl;

    cout << "DISTANCES\nlocation: customers (distance)\n";
    for (auto loc:p_locations) {
        cout << loc << ": ";
        for (auto cust:instance->getCustomers()) {
            if (assignment[cust].node == loc) {
                cout << cust << " ";
                cout << "(" << instance->getWeightedDist(loc, cust) << ") ";
            }
        }
        cout << endl;
    }


    cout.rdbuf(stream_buffer_cout);
    file.close();
}


// Solution_std Solution_std::rand_swap2_Locations(Solution_std sol_current){

//     if (sol_current.get_pLocations().size() < 2) return sol_current;


//     // swap 2 locations
//     auto locations = sol_current.get_Locations();
//     auto p_locations = sol_current.get_pLocations();

//     vector<uint_t> p_locations_vec;
//     p_locations_vec.reserve(p_locations.size());
//     for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

//     vector<uint_t> out_locations_vec;
//     out_locations_vec.reserve(locations.size() - p_locations.size());
//     for (auto loc:locations) {
//         if (!p_locations.contains(loc)) {
//             out_locations_vec.push_back(loc);
//         }
//     }

//     if (out_locations_vec.size() < 2) return sol_current;

//     // Use a random_device to seed the random number generator
//     std::random_device rd;
    
//     // Use mt19937 as the random number generator engine
//     std::mt19937 gen(rd());

//     std::uniform_int_distribution<uint_t> distribution(0, out_locations_vec.size() - 1);
//     uint_t index1 = distribution(gen);
//     uint_t index2 = distribution(gen);
//     // Ensure the two indices are distinct
//     while (index2 == index1) {
//         index2 = distribution(gen);
//     }
//     uint_t loc1 = out_locations_vec[index1];
//     uint_t loc2 = out_locations_vec[index2];

//     // Use mt19937 as the random number generator engine
//     std::mt19937 gen2(rd());

//     std::uniform_int_distribution<uint_t> distribution2(0, out_locations_vec.size() - 1);
//     index1 = distribution2(gen2);
//     index2 = distribution2(gen2);
//     // Ensure the two indices are distinct
//     while (index2 == index1) {
//         index2 = distribution2(gen2);
//     }

//     uint_t p_loc1 = p_locations_vec[index1];
//     uint_t p_loc2 = p_locations_vec[index2];

//     Solution_std sol_tmp = sol_current;
//     sol_tmp.replaceLocation(p_loc1, loc1);
//     sol_tmp.replaceLocation(p_loc2, loc2);

//     if (sol_tmp.get_objective() < sol_current.get_objective()) {
//         sol_tmp.naiveEval();
//         return sol_tmp;
//     } else {
//         return sol_current;
//     }

//     return sol_current;
// }

