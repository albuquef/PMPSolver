
#include "solution_cap.hpp"
#include "globals.hpp"
#include "PMP.hpp"
#include <iomanip>
#include <utility>


Solution_cap::Solution_cap(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations, const char* typeEval, bool cover_mode) {
    
    this->instance = instance;
    this->p_locations = p_locations;
    // print p_locations    

    // cover_mode = instance->isCoverMode();

    // Initialize all fields
    // this->cover_mode = instance->isCoverMode();
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
                 unordered_map<uint_t, assignment> assignments, dist_t objective) {
    this->instance = std::move(instance);
    this->p_locations = std::move(p_locations);
    this->loc_usages = std::move(loc_usages);
    this->cust_satisfactions = std::move(cust_satisfactions);
    this->assignments = std::move(assignments);
    this->objective = objective;
    this->typeEval = "CPLEX";
    // objEval();
    // GAP_eval();
}


void Solution_cap::naiveEval() {
//    assert(p_locations.size() == instance->get_p());
    objective = 0;
    for (auto cust:instance->getCustomers()) {
        auto loc = getClosestpLoc(cust);
        auto dist = instance->getWeightedDist(loc, cust);
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
        dist_t dist = instance->getWeightedDist(loc, cust);
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
        fprintf(stderr, "Total capacity (%i) < total demand (%i)\n", total_capacity, total_demand);
        isFeasible = false;
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
                    // exit(1);
                    // break;
                }else{
                    auto cap_rem = instance->getLocCapacity(loc) - loc_usages[loc];
                    if (dem_rem > cap_rem) { // assign all remaining location capacity
                        loc_usages[loc] += cap_rem;
                        cust_satisfactions[cust] += cap_rem;
                        auto obj_increment = cap_rem * instance->getRealDist(loc, cust);
                        // objective += obj_increment;
                        assignments[cust].emplace_back(my_tuple{loc, cap_rem, obj_increment});
                        dem_rem -= cap_rem;
                        location_full = true;

                    //     cout << "Finish all capacited\n";
                    //    cout << "cust: " << cust << " dem_rem: " << dem_rem << endl;
                    //     cout << "loc: " << loc << " cap_rem: " << cap_rem << endl;
                    //     cout << "loc_usages[" << loc << "]: " << loc_usages[loc] << endl;
                    //     cout << "cust_satisfactions[" << cust << "]: " << cust_satisfactions[cust] << endl;
                    //     cout << "\n";

                        break;
                    } else { // assign dem_rem
                        loc_usages[loc] += dem_rem;
                        cust_satisfactions[cust] += dem_rem;
                        auto obj_increment = dem_rem * instance->getRealDist(loc, cust);
                        // objective += obj_increment;
                        assignments[cust].emplace_back(my_tuple{loc, dem_rem, obj_increment});
                        dem_rem = 0;
                    //    cout << "Finish all demand\n";
                    //    cout << "cust: " << cust << " dem_rem: " << dem_rem << endl;
                    //     cout << "loc: " << loc << " cap_rem: " << cap_rem << endl;
                    //     cout << "loc_usages[" << loc << "]: " << loc_usages[loc] << endl;
                    //     cout << "cust_satisfactions[" << cust << "]: " << cust_satisfactions[cust] << endl;
                    //     cout << "\n";
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
    objEval();

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
    if (!VERBOSE) return;
    
    cout << "p locations: ";
    for (auto p:p_locations) {
        cout << p << " ";
    }
    cout << "\np size: " << p_locations.size() << endl;
    cout << setprecision(15) << "objective: " << objective << endl;
    cout << "demand/capacity: " << instance->getTotalDemand() << "/" << getTotalCapacity() << endl;
    if(cover_mode){ cout << "cover mode: " <<  instance->getTypeSubarea() << "\n";
    }else{ cout << "cover mode: OFF" << "\n";}
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
    // is in the same subarea
    auto p_loc_cand = p_locations;
    p_loc_cand.erase(loc_old);
    p_loc_cand.insert(loc_new);
    if(instance->isCoverMode()){
        if(!instance->isPcoversAllSubareas(p_loc_cand)){
            cerr << "ERROR: p_locations do not cover all subareas" << endl;
            // cout << "p_locations: ";
            // for (auto p:p_locations) {
            //     cout << p << " ";
            // }
            // cout << endl;
            // exit(1);
        }
    }

}

dist_t Solution_cap::get_objective() const {
    return objective;
}

void Solution_cap::saveAssignment(string output_filename,string Method) {
    
    cout << "[INFO] Saving assignment" << endl;
    
    fstream file;
    streambuf *stream_buffer_cout = cout.rdbuf();

    string output_filename_final = output_filename + 
        "_p_" + to_string(p_locations.size()) + 
        "_" + Method +
        ".txt";

    if(cover_mode){
        output_filename_final = output_filename + 
        "_p_" + to_string(p_locations.size()) + 
        "_" + Method + "_cover_" + instance->getTypeSubarea() +
        ".txt";
    }

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

    cout.rdbuf(stream_buffer_cout);
    file.close();
}

void Solution_cap::saveResults(string output_filename, double timeFinal, int numIter,string Method, string Method_sp, string Method_fp){

    cout << "[INFO] Saving results" << endl;

    string output_filename_final = output_filename + 
    "_results_" + Method +
    ".csv";

    if(cover_mode){
        output_filename_final = output_filename + 
        "_results_" + Method + "_cover_" + instance->getTypeSubarea() +
        ".csv";
    }

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
        outputTable << typeEval << ";"; 
        outputTable << fixed << setprecision(15) << get_objective() << ";"; // obj value
        outputTable << numIter << ";"; // 
        outputTable << fixed << setprecision(15) << timeFinal <<  ";"; // time cplex
        outputTable << Method_sp << ";";
        outputTable << Method_fp << ";";
        outputTable << "\n";
        // if (Method_sp != "null") {outputTable << Method_sp << ";";}
        // if (Method_fp != "null") {outputTable << Method_fp << ";";}
        // outputTable << "\n";
    }
    outputTable.close();

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
            cout << "GAP not feasible" << endl;
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
    this->objective = 0;
    for (auto cust:instance->getCustomers()) {
        for (auto a:assignments[cust]) this->objective += a.usage * instance->getRealDist(a.node, cust);
    }

    // cout << "objective Eval: " << objective << endl;

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
            if (a.usage > instance->getLocCapacity(a.node)+0.01){
                if (verb) cout << "ERROR: usage > capacity" << endl;
                if (verb) cout << "usage: " << a.usage << "\n capacity: " << instance->getLocCapacity(a.node) << endl;
                isFeasible = false;
                return isFeasible;
                // exit(1);
            }
            if (a.usage > instance->getCustWeight(cust) + 0.01){
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

    if(cover_mode){
        // auto coverages = instance->getSubareasSet();
        for(auto subarea:instance->getSubareasSet()){
            auto loc_subarea = instance->getLocationsSubarea(subarea);
            bool covered = false;
            for(auto loc:loc_subarea){
                if (p_locations.find(loc) != p_locations.end()){
                    covered = true;
                }
            }
            if (!covered){
                if (verb) cout << "ERROR: subarea not covered" << endl;
                if (verb) cout << "subarea: " << subarea << endl;
                isFeasible = false;
                return isFeasible;
            }

        }
    }
 
    return isFeasible;
}

bool Solution_cap::isCoverMode(){
    return cover_mode;
}

void Solution_cap::setCoverMode(bool cover_mode){
    this->cover_mode = cover_mode;
}

void Solution_cap::add_UpperBound(double UB){
    this->UpperBound = UB;
}   