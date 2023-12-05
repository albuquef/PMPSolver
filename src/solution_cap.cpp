//
// Created by david on 09/06/2021.
//

#include "solution_cap.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <iomanip>
#include <utility>

Solution_cap::Solution_cap(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations) {
    this->instance = std::move(instance);
    this->p_locations = std::move(p_locations);
    fullCapEval();
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
    objEval();
}



vector<pair<uint_t, dist_t>> Solution_cap::getUrgencies() {
    vector<pair<uint_t, dist_t>> urgencies_vec;

    // get closest and second closest p location with some remaining capacity
    for (auto p:cust_satisfactions) {
        auto cust = p.first;
        auto sat = p.second;                                // cust satisfaction
        auto dem = instance->getCustWeight(cust) - sat;     // cust remaining demand
        if (sat < dem) { // cust not fully satisfied yet
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
        exit(1);
    }


    // Determine unassigned customer's urgencies
    auto urgencies_vec = getUrgencies();
    bool location_full = false;
    bool infeasible = false;
    int cont = 0;
    while (!urgencies_vec.empty() && !infeasible) {
        // Assign customers, until some capacity is full
        for (auto p:urgencies_vec) {
            auto cust = p.first;
            auto dem_rem = instance->getCustWeight(cust) - cust_satisfactions[cust]; // remaining demand
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
                        objective += obj_increment;
                        assignments[cust].emplace_back(my_tuple{loc, cap_rem, obj_increment});
                        dem_rem -= cap_rem;
                        location_full = true;
                        break;
                    } else { // assign dem_rem
                        loc_usages[loc] += dem_rem;
                        cust_satisfactions[cust] += dem_rem;
                        auto obj_increment = dem_rem * instance->getRealDist(loc, cust);
                        objective += obj_increment;
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

    cout << "fullCapEval: " << objective << endl;
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
    cout << endl;
    cout << setprecision(15) << "objective: " << objective << endl;
    cout << "demand/capacity: " << instance->getTotalDemand() << "/" << getTotalCapacity() << endl;
}

const unordered_set<uint_t> &Solution_cap::get_pLocations() const {
    return this->p_locations;
}

void Solution_cap::replaceLocation(uint_t loc_old, uint_t loc_new) {
    // Update p_locations
    p_locations.erase(loc_old);
    p_locations.insert(loc_new);
    // Update assignment and objective
    fullCapEval();

    checkClock();
}

dist_t Solution_cap::get_objective() const {
    return objective;
}

void Solution_cap::saveAssignment(string output_filename,int mode) {
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

uint_t Solution_cap::getTotalCapacity() {
    uint_t total_cap = 0;
    for (auto p_loc:p_locations) total_cap += instance->getLocCapacity(p_loc);
    return total_cap;
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

// void Solution_cap::setSolution(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations
//                     ,unordered_map<uint_t, uint_t> loc_usages, unordered_map<uint_t, uint_t> cust_satisfactions
//                     ,unordered_map<uint_t, assignment> assignments){
//     this->instance = instance;
//     this->p_locations = p_locations;
//     this->loc_usages = loc_usages;
//     this->cust_satisfactions = cust_satisfactions;
//     this->assignments = assignments;
//     objEval();
// }



void Solution_cap::objEval(){
    objective = 0;
    for (auto cust:instance->getCustomers()) {
        for (auto a:assignments[cust]) objective += a.usage * instance->getRealDist(a.node, cust);
    }

    cout << "objective Eval: " << objective << endl;

}

