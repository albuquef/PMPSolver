#include <iomanip>
#include <utility>
#include <experimental/filesystem>
#include "solution_std.hpp"

Solution_std::Solution_std(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations) {
    
    this->instance = instance;
    this->p_locations = p_locations;

    // instance->print();

    naiveEval();
}

void Solution_std::naiveEval() {
//    assert(p_locations.size() == instance->get_p());

    objective = 0;
    for (auto cust:instance->getCustomers()) {
        auto loc = getClosestpLoc(cust);
        auto dist = instance->getWeightedDist(loc, cust);
        // cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
        objective += dist;
        assignment[cust] = my_pair{loc, dist};
//        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
    }
}

uint_t Solution_std::getClosestpLoc(uint_t cust) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    uint_t loc_closest=numeric_limits<uint_t>::max();
    for (auto loc:p_locations) {
        dist_t dist = instance->getWeightedDist(loc, cust);
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
    cout << endl;
    cout << "p size: " << p_locations.size() << endl;
    cout << setprecision(15) << "objective: " << objective << endl;
    if(cover_mode){ cout << "cover mode: " <<  instance->getTypeSubarea() << "\n";
    }else{ cout << "cover mode: OFF" << "\n";}
    if(cover_mode_n2){ cout << "cover mode n2: " <<  instance->getTypeSubarea_n2() << "\n";
    }else{ cout << "cover mode n2: OFF" << "\n";}
    // solution feasible
    if(isSolutionFeasible()) cout << "Solution is Feasible\n";
    else cout << "Solution is Infeasible\n";
    cout << "\n";
}

const unordered_set<uint_t> &Solution_std::get_pLocations() const {
    return this->p_locations;
}

const vector<uint_t> &Solution_std::get_Locations() const {
    return this->instance->getLocations();
}

void Solution_std::replaceLocation(uint_t loc_old, uint_t loc_new) {
    // Update p_locations

     if(!(p_locations.find(loc_old) == p_locations.end()) && !(p_locations.find(loc_new) != p_locations.end())){
    p_locations.erase(loc_old);
    p_locations.insert(loc_new);

    naiveEval();
     
    }else{
        cout << "ERROR: loc_old not in p_locations or loc_new in p_locations" << endl;
        
    }
//     // Update assignment and objective
//     for (auto cust:instance->getCustomers()) {
//         auto dist_old = assignment[cust].dist;
//         auto dist_new = instance->getWeightedDist(loc_new, cust);
//         if (assignment[cust].node == loc_old) { // cust must be reassigned to some other p location
//             auto loc_closest = getClosestpLoc(cust);
//             auto dist_closest = instance->getWeightedDist(loc_closest, cust);
//             assignment[cust] = my_pair {loc_closest, dist_closest};
//             objective -= dist_old;
//             objective += dist_closest;
//         } else if (dist_old - dist_new > TOLERANCE_OBJ) { // cust may be reassigned to loc_new
//             assignment[cust] = my_pair {loc_new, dist_new};
//             objective -= dist_old;
//             objective += dist_new;
//         }
// //        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
//     }
}

dist_t Solution_std::get_objective() {
    return objective;
}

void Solution_std::saveAssignment(string output_filename,string Method) {

    cout << "[INFO] Saving Assignment pmp" << endl;

    fstream file;
    streambuf *stream_buffer_cout = cout.rdbuf();
    string output_filename_final = output_filename + 
        "_p_" + to_string(p_locations.size()) + 
        "_" + Method +
        ".txt";

    // Open file if output_filename is not empty
    if (!output_filename_final.empty()) {
        file.open(output_filename_final, ios::out);
        streambuf *stream_buffer_file = file.rdbuf();
        cout.rdbuf(stream_buffer_file); // redirect cout to file
    }

    cout << setprecision(15) << "OBJECTIVE\n" << objective << endl << endl;

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


void Solution_std::saveResults(string output_filename, double timeFinal, int numIter,string Method, string Method_sp, string Method_fp){

    cout << "[INFO] Saving results pmp" << endl;

    string output_filename_final = output_filename + 
    "_results_" + Method +
    ".csv";

    ofstream outputTable;
    outputTable.open(output_filename_final,ios:: app);

    if (!outputTable.is_open()) {
        cerr << "Error opening file: " << output_filename << endl;
        // return;
    }else{
        outputTable << instance->getCustomers().size() << ";";
        outputTable << instance->getLocations().size() << ";";
        outputTable << instance->get_p() << ";";
        outputTable << Method << ";";
        outputTable << fixed << setprecision(15) << get_objective() << ";"; // obj value
        outputTable << numIter << ";"; // 
        outputTable << timeFinal <<  ";"; // time cplex
        outputTable << Method_sp << ";";
        outputTable << Method_fp << ";";
        outputTable << "\n";
        // if (Method_sp != "null") {outputTable << Method_sp << ";";}
        // if (Method_fp != "null") {outputTable << Method_fp << ";";}
        // outputTable << "\n";
    }
    outputTable.close();
}


bool Solution_std::isSolutionFeasible() {
    bool feas = true;
    if (p_locations.size() != instance->get_p()) return false;
    if(cover_mode) feas = instance->isPcoversAllSubareas(p_locations);
    if(feas && cover_mode_n2) return instance->isPcoversAllSubareas_n2(p_locations);

    return feas;
}