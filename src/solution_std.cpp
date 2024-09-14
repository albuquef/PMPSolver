#include <iomanip>
#include <utility>
#include <experimental/filesystem>
#include <cstring>
#include "solution_std.hpp"

Solution_std::Solution_std(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations) {
    
    this->instance = instance;
    this->p_locations = p_locations;

    // instance->print();

    is_weighted_obj_func = instance->get_isWeightedObjFunc();


    naiveEval();
}

void Solution_std::naiveEval() {
//    assert(p_locations.size() == instance->get_p());
    
    objective = 0;
    for (auto cust:instance->getCustomers()) {
        auto loc = getClosestpLoc(cust);
        
        auto dist = instance->getRealDist(loc, cust);
        if (is_weighted_obj_func){dist = instance->getWeightedDist(loc, cust);}

        // auto dist = instance->getWeightedDist(loc, cust);

        objective += dist;
        assignment[cust] = my_pair{loc, dist};
    }
}

uint_t Solution_std::getClosestpLoc(uint_t cust) {

    dist_t dist_min = numeric_limits<dist_t>::max();
    uint_t loc_closest=numeric_limits<uint_t>::max();
    for (auto loc:p_locations) {
        
        dist_t dist =  instance->getRealDist(loc, cust);
        if (is_weighted_obj_func){dist = instance->getWeightedDist(loc, cust);}


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
    if(instance->get_isWeightedObjFunc()){
        cout << "sum (wi * dij * xij) " << endl;
    }else{
        cout << "sum (dij * xij) " << endl;
    }
    
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

void Solution_std::saveAssignment(string output_filename,string Method, double timeFinal) {

    fstream file;
    streambuf *stream_buffer_cout = cout.rdbuf();

    string delimiter = "/";
    string directory;
    string rem_filename;

    // Find the last occurrence of the delimiter
    size_t pos = output_filename.find_last_of(delimiter);
    if (pos != std::string::npos) {
        // Extract the substring up to and including the last delimiter
        directory = output_filename.substr(0, pos + 1);
        rem_filename = output_filename.substr(pos + 1);
    } else {
        std::cerr << "[WARN] Delimiter not found in the filename string" << std::endl;
    }

    string output_filename_final = directory + "Assignments/" + rem_filename + 
        "_p_" + to_string(p_locations.size()) + 
        "_" + Method;
    if(cover_mode){output_filename_final +=  "_cover_" + instance->getTypeSubarea();}
    output_filename_final += ".txt";

    cout << "[INFO] Saving assignment: " << output_filename_final << endl;

    // Open file if output_filename is not empty
    if (!output_filename_final.empty()) {
        file.open(output_filename_final, ios::out);
        streambuf *stream_buffer_file = file.rdbuf();
        cout.rdbuf(stream_buffer_file); // redirect cout to file
    }


    // Infos
    cout << "INFOS\n";
    cout << "instance: " << instance->getTypeService() << endl;
    cout << "p: " << instance->get_p() << endl;
    if(instance->get_isWeightedObjFunc())
        cout << "Objective Function: Weighted" << endl;
    else
        cout << "Objective Funtion: Unweighted" << endl;

    if(cover_mode){cout << "Cover N1: " << instance->getTypeSubarea() << endl;}
    if(cover_mode_n2){cout << "Cover N2: " << instance->getTypeSubarea_n2() << endl;}
    cout << "Final time: " << timeFinal << endl;
    cout << endl;


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
    cout << endl;
    // stats
    cout << "STATS\n";
    cout << "max dist: " << max_dist << endl;
    cout << "min dist: " << min_dist << endl;
    cout << "avg dist: " << avg_dist << endl;
    cout << "std dev dist: " << std_dev_dist << endl;

    cout.rdbuf(stream_buffer_cout);
    file.close();
}


void Solution_std::saveResults(string output_filename, double timeFinal, int numIter,string Method, string Method_sp, string Method_fp){

    cout << "[INFO] Saving results pmp" << endl;

    string output_filename_final = output_filename + "_results_" + Method;
    if(cover_mode){output_filename_final += "_cover_" + instance->getTypeSubarea();}
    output_filename_final += ".csv";

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
    // outputTable.close();

    string output_filename_all = "./outputs/solutions/test_all_results.csv";
    cout << "[INFO] Saving all results: "  << output_filename_all << endl;
    
    ofstream outputTable_all;
    outputTable_all.open(output_filename_all,ios:: app);
    
    if (!outputTable_all.is_open()) {
        cerr << "Error opening file: " << output_filename_all << endl;
        // return;
    }else{
        // add the date and hour of the execution
        time_t now = time(0);
        tm *ltm = localtime(&now);
        outputTable_all << 1900 + ltm->tm_year << "-" << 1 + ltm->tm_mon << "-" << ltm->tm_mday << ";";
        outputTable_all << instance->getCustomers().size() << ";";
        outputTable_all << instance->getLocations().size() << ";";
        outputTable_all << instance->get_p() << ";";
        if (instance->get_isWeightedObjFunc()) outputTable_all << "weighted_obj" << ";";
        else outputTable_all << "non-weighted_obj" << ";";
        if (cover_mode) outputTable_all << instance->getTypeSubarea() << ";";
        else outputTable_all << "non-cover_mode" << ";";
        if (cover_mode_n2) outputTable_all << instance->getTypeSubarea_n2() << ";";
        else outputTable_all << "non-cover_mode_n2" << ";";
        // outputTable_all << instance->get_isWeightedObjFunc() << ";";
        // outputTable_all << instance->isCoverMode() << ";";
        // outputTable_all << instance->isCoverMode_n2() << ";";
        outputTable_all << instance->getTypeService() << ";";
        outputTable_all << instance->getTypeSubarea() << ";"; 
        outputTable_all << Method << ";";
        outputTable_all << fixed << setprecision(15) << get_objective() << ";"; // obj value
        outputTable_all << fixed << setprecision(15) << timeFinal <<  ";"; // time cplex
        outputTable_all << numIter << ";"; //
        if (strcmp(Method.c_str(), "RSSV") == 0){
            outputTable_all << Method_sp << ";";
            outputTable_all << Method_fp << ";"; 
            outputTable_all << SUB_PMP_SIZE << ";";
        }
        outputTable_all << "\n";
    }

    outputTable.close();
    outputTable_all.close();
}


bool Solution_std::isSolutionFeasible() {
    bool feas = true;
    if (p_locations.size() != instance->get_p()) return false;
    if(cover_mode) feas = instance->isPcoversAllSubareas(p_locations);
    if(feas && cover_mode_n2) return instance->isPcoversAllSubareas_n2(p_locations);

    return feas;
}

void Solution_std::statsDistances() {
    dist_t sum = 0;
    for (auto cust:instance->getCustomers()) {
        sum += assignment[cust].dist;
        if (assignment[cust].dist > max_dist) max_dist = assignment[cust].dist;
        if (assignment[cust].dist < min_dist) min_dist = assignment[cust].dist;
    }
    avg_dist = sum / instance->getCustomers().size();
    for (auto cust:instance->getCustomers()) {
        std_dev_dist += pow(assignment[cust].dist - avg_dist, 2);
    }
    std_dev_dist = sqrt(std_dev_dist / instance->getCustomers().size());

    this->max_dist = max_dist;
    this->min_dist = min_dist;
    this->avg_dist = avg_dist;
    this->std_dev_dist = std_dev_dist;

    max_num_assignments = calculateMaxNumberAssignments();

}

dist_t Solution_std::getMaxDist() {
    return max_dist;
}
dist_t Solution_std::getMinDist() {
    return min_dist;
}
dist_t Solution_std::getAvgDist() {
    return avg_dist;
}
dist_t Solution_std::getStdDevDist() {
    return std_dev_dist;
}

uint_t Solution_std::getMaxNumberAssignments() {
    return max_num_assignments;
}

uint Solution_std::calculateMaxNumberAssignments() {
    uint max = 0;
    for (auto loc:p_locations) {
        uint count = 0;
        for (auto cust:instance->getCustomers()) {
            if (assignment[cust].node == loc) count++;
        }
        if (count > max) max = count;
    }
    return max;
}


bool Solution_std::capacitiesAssigmentRespected(const shared_ptr<Instance>& instance_total) {
    // check if the assignment respects the capacities of the locations
    int cont = 0;
    bool cap_feasible = true;
    for (auto loc:p_locations) {
        uint_t sum_demands = 0;
        dist_t biggest_capacity_in_cluster = instance->getLocCapacity(loc);
        bool cluster_feasible = true;
        for (auto cust:instance->getCustomers()) {
            if (assignment[cust].node == loc){
                sum_demands += instance->getCustDemand(cust); 
                if (instance_total->getLocCapacity(cust) > biggest_capacity_in_cluster){
                    biggest_capacity_in_cluster = instance_total->getLocCapacity(cust);
                }             
            }
        }
        if (sum_demands <= biggest_capacity_in_cluster){
            cont++;
        }



        if (sum_demands > instance->getLocCapacity(loc)){
            cap_feasible = false;
        }

        // if (sum_demands > instance->getLocCapacity(loc)) return false;
        // check if there a cust that is also in loc and have the biggest capacity
        
    }

    cout << "Number of feasible clusters in the solution: " << cont << endl;

    return cap_feasible;
    // return true;
}
