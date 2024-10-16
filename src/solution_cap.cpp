
#include "solution_cap.hpp"
#include "globals.hpp"
#include "PMP.hpp"
#include <iomanip>
#include <utility>
#include <experimental/filesystem>


Solution_cap::Solution_cap(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations, const char* typeEval, bool cover_mode) {
    
    
    this->instance = instance;
    this->p_locations = p_locations;
    this->cover_mode = cover_mode;
    this->typeEval = typeEval;

    // cout << "typeEval: " << typeEval << endl;
    if (strcmp(typeEval, "GAP") == 0 || strcmp(typeEval, "GAPrelax") == 0){
        GAP_eval(); 
    }else if(strcmp(typeEval, "heuristic") == 0){
        fullCapEval(); // urgency priority heuristic
    }else{
        cerr << "ERROR: typeEval not recognized" << endl;
        // exit(1);
    }
}

Solution_cap::Solution_cap(shared_ptr<Instance> instance,
                 unordered_set<uint_t> p_locations,
                 unordered_map<uint_t, dist_t> loc_usages, 
                 unordered_map<uint_t, dist_t> cust_satisfactions, 
                 unordered_map<uint_t, assignment> assignments) {
    this->instance = std::move(instance);
    this->p_locations = std::move(p_locations);
    this->loc_usages = std::move(loc_usages);
    this->cust_satisfactions = std::move(cust_satisfactions);
    this->assignments = std::move(assignments);
    this->typeEval = "CPLEX";
    objEval();
    // GAP_eval();
}


void Solution_cap::naiveEval() {
//    assert(p_locations.size() == instance->get_p());
    objective = 0;
    for (auto cust:instance->getCustomers()) {
        auto loc = getClosestpLoc(cust);
        dist_t dist =  instance->getRealDist(loc, cust);
        if(instance->get_isWeightedObjFunc()) dist = instance->getWeightedDist(loc, cust);
        objective += dist;
        assignments[cust].emplace_back(my_tuple{loc, 0, dist});
        // assignment[cust] = my_pair{loc, dist};
//        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
    }
}

uint_t Solution_cap::getClosestpLoc(uint_t cust) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    uint_t loc_closest = numeric_limits<uint_t>::max();
    for (auto loc:p_locations) {
        dist_t dist =  instance->getRealDist(loc, cust);
        if(instance->get_isWeightedObjFunc()) dist = instance->getWeightedDist(loc, cust);
        if (dist <= dist_min) {
            dist_min = dist;
            loc_closest = loc;
        }
    }

    return loc_closest;
}

// Function to check if the target value is in the first element of any pair in the vector
bool isFirstValuePresent(const std::vector<std::pair<uint_t, dist_t>>& urgencies_vec, uint_t target) {
    // Iterate over each pair in the vector
    for (const auto& pair : urgencies_vec) {
        // Check if the target value matches the first element of the pair
        if (pair.first == target) {
            // Return true if the target value is found
            return true;
        }
    }
    // Return false if the target value is not found
    return false;
}
vector<pair<uint_t, dist_t>> Solution_cap::getUrgencies() {
    vector<pair<uint_t, dist_t>> urgencies_vec;

    // get closest and second closest p location with some remaining capacity
    for (auto p:cust_satisfactions) {
        auto cust = p.first;
        auto sat = p.second; // cust satisfaction                              
        // auto dem = instance->getCustWeight(cust) - sat;     // cust remaining demand
        auto dem = instance->getCustWeight(cust);     // cust remaining demand

        // if (sat < dem) { // cust not fully satisfied yet
        if (dem - sat > 0) { // cust not fully satisfied yet
            auto l1 = getClosestOpenpLoc(cust, numeric_limits<uint_t>::max());
            auto l2 = getClosestOpenpLoc(cust, l1);
            auto dist1 = instance->getRealDist(l1, cust);
            auto dist2 = instance->getRealDist(l2, cust);
            dist_t urgency = fabs(dist1 - dist2);
            urgencies_vec.emplace_back(make_pair(cust, urgency));
        }
    }

    // Sort customers by decreasing urgencies
    sort(urgencies_vec.begin(), urgencies_vec.end(), cmpPair2nd);
    reverse(urgencies_vec.begin(), urgencies_vec.end()); // high to low now

    return urgencies_vec;
}

void Solution_cap::fullCapEval() {
    // Initialize all fields
    bool is_weighted_obj_func = instance->get_isWeightedObjFunc();
    objective = 0;
    for (auto p_loc:this->p_locations) loc_usages[p_loc] = 0;
    for (auto cust:this->instance->getCustomers()) {
        cust_satisfactions[cust] = 0;
        assignments[cust] = assignment{};
    }

    // Check if capacity demands can be met
    uint_t total_capacity = 0;
    uint_t total_demand = instance->getTotalDemand();
    for (auto p_loc:p_locations) total_capacity += instance->getLocCapacity(p_loc);
    if (total_capacity < total_demand) {
        cout << "[ERROR] Total capacity < total demand" << endl;
        isFeasible = false;
        objective = numeric_limits<dist_t>::max();
        // exit(1);
        return;
    }

    if (p_locations.size() != instance->get_p()) {
        cout << "[ERROR] p_locations.size() != instance->get_p()" << endl;
        isFeasible = false;
        objective = numeric_limits<dist_t>::max();
        // exit(1);
        return;
    }


    // Determine unassigned customer's urgencies
    auto urgencies_vec = getUrgencies();
    bool location_full = false;
    bool infeasible = false;
    int cont = 0;
    int cont_iter = 0;

    while (!urgencies_vec.empty() && !infeasible) {
        // Assign customers, until some capacity is full
        for (auto p:urgencies_vec) {
            auto cust = p.first;
            auto dem_rem = instance->getCustWeight(cust) - cust_satisfactions[cust]; // remaining demand

            cont_iter++;
            // cout << "cont_iter: " << cont_iter << endl;
            // cout << "\n\ncust: " << cust << " dem_rem: " << dem_rem << endl;

            while (dem_rem > 0  && !infeasible) {
                auto loc = getClosestOpenpLoc(cust, numeric_limits<uint_t>::max());
                if (loc == numeric_limits<uint_t>::max()) {
                    cerr << "Assignment not possible\n";
                    infeasible = true;
                    cont++; 
                    cout << "cont: " << cont << endl;
                    objective = numeric_limits<dist_t>::max();
                    // exit(1);
                    // break;
                }else{
                    auto cap_rem = instance->getLocCapacity(loc) - loc_usages[loc];
                    if (dem_rem > cap_rem) { // assign all remaining location capacity
                        loc_usages[loc] += cap_rem;
                        cust_satisfactions[cust] += cap_rem;
                        
                        
                        auto obj_increment =  instance->getRealDist(loc, cust);
                        if(is_weighted_obj_func){obj_increment =  cap_rem * instance->getRealDist(loc, cust);}

                        // objective += obj_increment;
                        assignments[cust].emplace_back(my_tuple{loc, cap_rem, obj_increment});
                        dem_rem -= cap_rem;
                        location_full = true;

                        break;
                    } else { // assign dem_rem
                        loc_usages[loc] += dem_rem;
                        cust_satisfactions[cust] += dem_rem;
                        auto obj_increment = instance->getRealDist(loc, cust);
                        if(is_weighted_obj_func){obj_increment =  dem_rem * instance->getRealDist(loc, cust);}
                        // objective += obj_increment;
                        assignments[cust].emplace_back(my_tuple{loc, dem_rem, obj_increment});
                        dem_rem = 0;
                    }
                }
            }
            if (location_full) break;
            if (infeasible) break;
        }

        // Recompute urgencies and repeat (for unassigned customers and open locations only)
        location_full = false;
        urgencies_vec = getUrgencies();

    }

    isFeasible = !infeasible;
    // cout << "fullCapEval: " << objective << endl;
    if (isFeasible) objEval();
    else objective = numeric_limits<dist_t>::max();
    

    if (p_locations.size() != instance->get_p()) {
        cout << "[ERROR] p_locations.size() != instance->get_p()" << endl;
        isFeasible = false;
        objective = numeric_limits<dist_t>::max();
        // exit(1);
        return;
    }



}

uint_t Solution_cap::getClosestOpenpLoc(uint_t cust, uint_t forbidden_loc) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    dist_t dist;
    uint_t loc_closest = numeric_limits<uint_t>::max();
    for (auto loc:p_locations) {
        dist = instance->getRealDist(loc, cust);
        if (dist <= dist_min && loc_usages[loc] < instance->getLocCapacity(loc) && loc != forbidden_loc) {
            dist_min = dist;
            loc_closest = loc;
        }
    }
    return loc_closest;
}

void Solution_cap::print() {
    // if (!VERBOSE) return;
    
    cout << "p locations: ";
    for (auto p:p_locations) {
        cout << p << " ";
    }
    cout << "\np size: " << p_locations.size() << endl;
    cout << setprecision(15) << "objective: " << objective << endl;
    if(instance->get_isWeightedObjFunc()){
        cout << "sum (wi * dij * xij) " << endl;
    }else{
        cout << "sum (dij * xij) " << endl;
    }
    cout << "demand/capacity: " << instance->getTotalDemand() << "/" << getTotalCapacity() << endl;

    if(cover_mode){ cout << "cover mode: " <<  instance->getTypeSubarea() << "\n";}
    // else{ cout << "cover mode: OFF" << "\n";}
    if(cover_mode_n2){ cout << "cover mode n2: " <<  instance->getTypeSubarea_n2() << "\n";}
    // else{ cout << "cover mode n2: OFF" << "\n";}
    if(isSolutionFeasible()) cout << "Solution is Feasible\n";
    else cout << "Solution is Infeasible\n";
    
    // cout << "\n";
}

const unordered_set<uint_t> &Solution_cap::get_pLocations() const {
    return this->p_locations;
}

void Solution_cap::replaceLocation(uint_t loc_old, uint_t loc_new, const char* typeEVAL) {
    
    this->typeEval = typeEVAL;

    // test if loc_new is in p_locations or loc_old is not in p_locations
    if((p_locations.find(loc_old) != p_locations.end()) && (p_locations.find(loc_new) == p_locations.end())){
        // Update p_locations
        p_locations.erase(loc_old);
        p_locations.insert(loc_new);

        if (strcmp(typeEVAL, "GAP") == 0 || strcmp(typeEVAL, "GAPrelax") == 0){
            GAP_eval(); 
        }else if(strcmp(typeEVAL, "heuristic") == 0){
            fullCapEval(); // urgency priority heuristic
        }else if (strcmp(typeEVAL, "naive") == 0 || strcmp(typeEVAL, "PMP") == 0){
            naiveEval();
        }else{
            cerr << "ERROR: typeEVAL not recognized" << endl;
            // exit(1);
        }
    }else{
        cerr << "ERROR: loc_new already in p_locations or loc_old not in p_locations" << endl;
        if(!(p_locations.find(loc_old) != p_locations.end())) {
            cerr << "loc_old: " << loc_old << " not in p locations" << endl;
        }
        if(!(p_locations.find(loc_new) == p_locations.end())) {
            cerr << "loc_new: " << loc_new << " is in p_locations" << endl;
        }
        cout << "loc_old: " << loc_old << " loc_new: " << loc_new << endl;
        cout << "p_locations: ";
        for (auto p:p_locations) {
            cout << p << " ";
        }
        cout << endl;
        // exit(1);
    } 


}

dist_t Solution_cap::get_objective() const {
    return objective;
}

void Solution_cap::saveAssignment(string output_filename, string Method, double timeFinal) {
    
    // cout << "[INFO] Saving assignment" << endl;
    
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

    cout << "LOCATION USAGES\nlocation (usage/capacity)\n";
    for (auto p_loc:p_locations)
        cout << p_loc << " (" << loc_usages[p_loc] << "/" << instance->getLocCapacity(p_loc) << ")\n";
    cout << endl;

    cout << "CUSTOMER ASSIGNMENTS\ncustomer (demand) -> location (assigned demand)\n";
    for (auto cust:instance->getCustomers()) {
        cout << cust << " (" << instance->getCustWeight(cust) << ") -> ";
        for (auto a:assignments[cust]) cout << a.node << " (" << a.usage << ") ";
        cout << endl;
    }
    cout << endl;

    // stats
    cout << "STATS\n";
    cout << "max_dist: " << max_dist << endl;
    cout << "min_dist: " << min_dist << endl;
    cout << "avg_dist: " << avg_dist << endl;
    cout << "std_dev_dist: " << std_dev_dist << endl;

    cout.rdbuf(stream_buffer_cout);
    file.close();
}

void Solution_cap::saveResults(string output_filename, double timeFinal, int numIter,string Method, string Method_sp, string Method_fp){


    string output_filename_final = output_filename + "_results_" + Method;
    if(cover_mode){output_filename_final += "_cover_" + instance->getTypeSubarea();}
    output_filename_final += ".csv";

    cout << "[INFO] Saving results: "  << output_filename_final << endl;

    ofstream outputTable;
    outputTable.open(output_filename_final,ios:: app);

    if (!outputTable.is_open()) {
        cerr << "Error opening file: " << output_filename_final << endl;
        // return;
    }else{
        time_t now = time(0);
        tm *ltm = localtime(&now);
        outputTable << 1900 + ltm->tm_year << "-" << 1 + ltm->tm_mon << "-" << ltm->tm_mday << ";";
        outputTable << instance->getCustomers().size() << ";";
        outputTable << instance->getLocations().size() << ";";
        outputTable << instance->get_p() << ";";
        if (instance->get_isWeightedObjFunc()) outputTable << "weighted_obj" << ";";
        else outputTable << "non-weighted_obj" << ";";
        if (cover_mode) outputTable << instance->getTypeSubarea() << ";";
        else outputTable << "non-cover_mode" << ";";
        if (cover_mode_n2) outputTable << instance->getTypeSubarea_n2() << ";";
        else outputTable << "non-cover_mode_n2" << ";";
        outputTable << instance->getTypeService() << ";";
        outputTable << instance->getTypeSubarea() << ";";
        outputTable << typeEval << ";"; 
        outputTable << Method << ";";
        outputTable << fixed << setprecision(15) << get_objective() << ";"; // obj value
        outputTable << fixed << setprecision(15) << timeFinal <<  ";"; // time cplex
        outputTable << numIter << ";"; //
        outputTable << Method_sp << ";";
        outputTable << Method_fp << ";"; 
        outputTable << SUB_PMP_SIZE << ";";
        outputTable << "\n";
    }
    // outputTable.close();


    // SAVE RESULTS ALL 
    string output_filename_all = "./outputs/solutions/test_all_results.csv";
    cout << "[INFO] Saving all results: "  << output_filename_all << endl;
    
    ofstream outputTable_all;
    outputTable_all.open(output_filename_all,ios:: app);

    // bool fileIsEmpty = false;
    // if (!filesystem::exists(output_filename_all)) {
    //     fileIsEmpty = true;
    // } else {
    //     // Check if the file is empty
    //     fileIsEmpty = (filesystem::file_size(output_filename_all) == 0);
    // }

    // if (fileIsEmpty) {
    //     outputTable_all << "date;num_cust;num_loc;p;isWeightedObjFunc;cover_mode;cover_mode_n2;type_service;type_subarea;typeEval;Method;solution;time;numIter;Method_sp;Method_fp;SUB_PMP_SIZE\n";
    // }


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
        outputTable_all << typeEval << ";"; 
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


uint_t Solution_cap::getTotalCapacity() {
    uint_t total_cap = 0;
    for (auto p_loc:p_locations) total_cap += instance->getLocCapacity(p_loc);
    return total_cap;
}

unordered_map<uint_t, dist_t>  Solution_cap::getLocUsages(){
    return loc_usages;
}
unordered_map<uint_t, dist_t> Solution_cap::getCustSatisfactions(){
    return cust_satisfactions;
}
unordered_map<uint_t, assignment> Solution_cap::getAssignments(){
    return assignments;
}


void Solution_cap::setLocUsage(uint_t loc, dist_t usage){

    if (usage > instance->getLocCapacity(loc)){
        cerr << "ERROR: usage > capacity" << endl;
        exit(1);
    }   
    loc_usages[loc] = usage;
    objEval();
}

void Solution_cap::setCustSatisfaction(uint_t cust, dist_t satisfaction){

    if (satisfaction > instance->getCustWeight(cust)){
        cerr << "ERROR: satisfaction > weight" << endl;
        exit(1);
    }   
    cust_satisfactions[cust] = satisfaction;
    objEval();
}

void Solution_cap::setAssigment(uint_t cust, assignment assigment){
    assignments[cust] = assigment;
    objEval();
}

void Solution_cap::setSolution(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations
                    ,unordered_map<uint_t, dist_t> loc_usages, unordered_map<uint_t, dist_t> cust_satisfactions
                    ,unordered_map<uint_t, assignment> assignments, dist_t objective){
    this->instance = instance;
    this->p_locations = p_locations;
    this->loc_usages = loc_usages;
    this->cust_satisfactions = cust_satisfactions;
    this->assignments = assignments;
    this->objective = objective;    
    // objEval();
}

void Solution_cap::GAP_eval(){
    // Initialize all fields
    // bool is_weighted_obj_func = instance->get_isWeightedObjFunc();;
    objective = 0;
    for (auto p_loc:this->p_locations) loc_usages[p_loc] = 0;
    for (auto cust:this->instance->getCustomers()) {
        cust_satisfactions[cust] = 0;
        assignments[cust] = assignment{};
    }

    if (strcmp(typeEval, "GAP") == 0){
        PMP pmp(instance, "GAP", true);
        if (UpperBound > 0) pmp.setUpperBound(UpperBound);
        // pmp.setCoverMode(cover_mode);
        pmp.run_GAP(p_locations);
        // auto sol_gap = pmp.getSolution_cap();
        if (pmp.getFeasibility_Solver()){
            isFeasible = true;  
            auto sol_gap = pmp.getSolution_cap();
            setSolution(instance, sol_gap.get_pLocations(), sol_gap.getLocUsages(),
                sol_gap.getCustSatisfactions(), sol_gap.getAssignments(), sol_gap.get_objective());
        }else{
            objective=numeric_limits<dist_t>::max();
            // cout << "GAP not feasible" << endl;
            auto sol_gap = Solution_cap();
            isFeasible = false;
        }
    }
    if (strcmp(typeEval, "GAPrelax") == 0){
        PMP pmp(instance, "GAP", false);
        // pmp.setCoverMode(cover_mode);
        if (UpperBound > 0) pmp.setUpperBound(UpperBound);
        pmp.run_GAP(p_locations);
        // auto sol_gap = pmp.getSolution_cap();
        if (pmp.getFeasibility_Solver()){
            isFeasible = true;
            auto sol_gap = pmp.getSolution_cap();
            // sol_gap.print();
            // sol_gap.saveAssignment("GAP_intern", "GAP");
            setSolution(instance, sol_gap.get_pLocations(), sol_gap.getLocUsages(),
                sol_gap.getCustSatisfactions(), sol_gap.getAssignments(), sol_gap.get_objective());
        }else{
            objective=numeric_limits<dist_t>::max();
            // cout << "GAPrelax not feasible" << endl;
            auto sol_gap = Solution_cap();
            isFeasible = false;
        }
    }
}

void Solution_cap::objEval(){

    bool is_weighted_obj_func = instance->get_isWeightedObjFunc();

    this->objective = 0;
    dist_t obj_value = 0.0;
    for (auto cust:instance->getCustomers()) {
        for (auto a:assignments[cust]){ 
            // a.usage = wi * xij 
            if(is_weighted_obj_func){obj_value += a.usage * instance->getRealDist(a.node, cust);}
            // xij = a.usage/wi
            else{obj_value += (a.usage/instance->getCustWeight(cust)) * instance->getRealDist(a.node, cust);}
        }
    }
    this->objective = obj_value;
}

bool Solution_cap::getFeasibility(){
    return isFeasible;
}

void Solution_cap::setFeasibility(bool feasible){
    isFeasible = feasible;
}


int getIndex(vector<uint_t> vec, uint_t val){
    auto it = find(vec.begin(), vec.end(), val);
    if (it != vec.end()) return distance(vec.begin(), it);
    else return -1;
}

bool Solution_cap::isSolutionFeasible(){

    // check if the p location is a vector and if is empty
    if (p_locations.empty()){
        isFeasible = false;
        return isFeasible;
    }


    isFeasible = true;
    bool verb = false;

    uint_t total_capacity = 0;
    uint_t total_demand = instance->getTotalDemand();
    for (auto p_loc:p_locations) total_capacity += instance->getLocCapacity(p_loc);
    if (total_capacity < total_demand) {
        if (verb) fprintf(stderr, "Total capacity (%i) < total demand (%i)\n", total_capacity, total_demand);
        isFeasible = false;
        // exit(1);
        return isFeasible;
    }
    
    auto locations = instance->getLocations();
    vector<uint_t> vector_capacities = vector<uint_t>(locations.size(), 0);

    for (auto cust:instance->getCustomers()) {
        double satisfaction = 0;
        for (auto a:assignments[cust]) {
            satisfaction += a.usage;
            vector_capacities[getIndex(locations, a.node)] += a.usage;
            if (a.usage > instance->getLocCapacity(a.node)+0.1){
                if (verb) cout << "ERROR: usage > capacity" << endl;
                if (verb) cout << "usage: " << a.usage << "\n capacity: " << instance->getLocCapacity(a.node) << endl;
                isFeasible = false;
                return isFeasible;
                // exit(1);
            }
            if (a.usage > instance->getCustWeight(cust) + 0.1){
                if (verb) cout << "ERROR: satisfaction > weight" << endl;
                if (verb) cout << "satisfaction: " << a.usage << "\n weight: " << instance->getCustWeight(cust) << endl;
                isFeasible = false;
                return isFeasible;
                // exit(1);
            }
        }
        if (satisfaction + 0.1 < instance->getCustWeight(cust)){
            if (verb) cout << "ERROR: Cust= " <<  cust << " not satisfied" << endl;
            if (verb) cout << "satisfaction: " << satisfaction << "\n weight: " << instance->getCustWeight(cust) << endl;
            isFeasible = false;
            return isFeasible;
            // exit(1);
        }
    }


    for (auto loc:instance->getLocations()) {
        if (vector_capacities[getIndex(locations,loc)] > instance->getLocCapacity(loc)){
            if (verb) cout << "ERROR: usage > capacity" << endl;
            isFeasible = false;
            return isFeasible;
            // exit(1);
        }
    }

    if(cover_mode){isFeasible = instance->isPcoversAllSubareas(p_locations);}
    if(cover_mode_n2){isFeasible = instance->isPcoversAllSubareas_n2(p_locations);}
 
    return isFeasible;
}

bool Solution_cap::isCoverMode(){
    return cover_mode;
}
bool Solution_cap::isCoverModeN2(){
    return cover_mode_n2;
}

void Solution_cap::setCoverMode(bool cover_mode){
    this->cover_mode = cover_mode;
}
void Solution_cap::setCoverMode_n2(bool cover_mode_n2){
    this->cover_mode_n2 = cover_mode_n2;
}


void Solution_cap::add_UpperBound(double UB){
    this->UpperBound = UB;
}   


void Solution_cap::statsDistances(){
    dist_t max_dist = 0;
    dist_t min_dist = numeric_limits<dist_t>::max();
    dist_t avg_dist = 0;
    dist_t dist;
    uint_t cont = 0;
    // bool is_weighted_obj_func = instance->get_isWeightedObjFunc();

    for (auto cust:instance->getCustomers()) {
        for (auto a:assignments[cust]) {
        
            
        //     // dist = instance->getRealDist(a.node, cust);
        //    if(is_weighted_obj_func){dist = a.usage * instance->getRealDist(a.node, cust);} // PACA LIMIT IN WITHOUT DIST not weighted
        //     // xij = a.usage/wi
        //     else{dist = (a.usage/instance->getCustWeight(cust)) * instance->getRealDist(a.node, cust);}
            dist = (a.usage/instance->getCustWeight(cust)) * instance->getRealDist(a.node, cust);

            if (dist > max_dist) max_dist = dist;
            if (dist < min_dist) min_dist = dist;
            avg_dist += dist;
            cont++;
                
            
        }
    }
    avg_dist = avg_dist/cont;
    this->max_dist = max_dist;
    this->min_dist = min_dist;
    this->avg_dist = avg_dist;
    // standard deviation
    dist_t std_dev_dist = 0;
    dist_t sum = 0;
    for (auto cust:instance->getCustomers()) {
        for (auto a:assignments[cust]) {
            dist = instance->getRealDist(a.node, cust);
            sum += pow(dist - avg_dist, 2);
        }
    }
    std_dev_dist = sqrt(sum/cont);
    this->std_dev_dist = std_dev_dist;

    this->max_num_assignments = calculateMaxNumberAssignments();

}

dist_t Solution_cap::getMaxDist(){
    return max_dist;
}
dist_t Solution_cap::getMinDist(){
    return min_dist;
}
dist_t Solution_cap::getAvgDist(){
    return avg_dist;
}
dist_t Solution_cap::getStdDevDist(){
    return std_dev_dist;
}

uint_t Solution_cap::getMaxNumberAssignments() {
    return max_num_assignments;
}

uint Solution_cap::calculateMaxNumberAssignments() {
    uint max = 0;
    for (auto loc:p_locations) {
        uint num_assignments = 0;
        for (auto cust:instance->getCustomers()) {
            for (auto a:assignments[cust]) {
                if (a.node == loc) num_assignments++;
            }
        }
        if (num_assignments > max) max = num_assignments;
    }
    return max;
}



void Solution_cap::printStatsDistances(){
    cout << "Statistics:\n";
    cout << "Max distance: " << getMaxDist() << endl;
    cout << "Min distance: " << getMinDist() << endl;
    cout << "Avg distance: " << getAvgDist() << endl;
    cout << "Std deviation distance: " << getStdDevDist() << endl;
    cout << endl << endl;
}


void Solution_cap::setBestBound(dist_t best_bound){
    this->Best_Bound = best_bound;
}
dist_t Solution_cap::getBestBound(){
    return this->Best_Bound;
}

bool Solution_cap::capacitiesAssigmentRespected(const shared_ptr<Instance>& instance_total){
    bool is_respected = true;
    for (auto loc:instance->getLocations()) {
        if (loc_usages[loc] > instance->getLocCapacity(loc)){
            is_respected = false;
            break;
        }

    }
    return is_respected;
}