#include "TB.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include <utility>

TB::TB(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
//    cout << "TB heuristic initialized\n";
//    instance->print();
}

Solution_std TB::initRandomSolution() {
    // Sample p distinct locations
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    uniform_int_distribution<uint_t> distribution (0, locations.size() - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = locations[loc_id];
        p_locations.insert(loc);
    }
    Solution_std sol(instance, p_locations);
    return sol;
}

Solution_cap TB::initRandomCapSolution() {
    // Sample p distinct locations
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    uniform_int_distribution<uint_t> distribution (0, locations.size() - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = locations[loc_id];
        p_locations.insert(loc);
    }
    Solution_cap sol(instance, p_locations);
    return sol;
}

Solution_cap TB::initHighestCapSolution() {
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();

    auto locations = instance->getLocations();

    vector<pair<uint_t, uint_t>> sorted_locations;
    for (auto loc:locations) {
        auto cap = instance->getLocCapacity(loc);
        sorted_locations.emplace_back(cap, loc);
    }
    sort(sorted_locations.begin(), sorted_locations.end());
    reverse(sorted_locations.begin(), sorted_locations.end());

    for (int i = 0; i < p; i++) {
        p_locations.insert(sorted_locations[i].second);
    }
    Solution_cap sol(instance, p_locations);

    return sol;
}


Solution_std TB::run(bool verbose) {
    checkClock();
    verbose = VERBOSE;

    auto sol_best = initRandomSolution();
    auto locations = instance->getLocations();
    bool improved = true;
    Solution_std sol_tmp;
    Solution_std sol_cand;
    int objectiveCpt = 0;

    while (improved) {
        checkClock();
        improved = false;
        sol_cand = sol_best;
        auto start = tick();
        auto p_locations = sol_best.get_pLocations();

        for (auto loc:locations) { // First improvement over locations
            if (!p_locations.contains(loc)) {
                for (auto p_loc:p_locations) { // Best improvement over p_locations
                    sol_tmp = sol_best;
                    sol_tmp.replaceLocation(p_loc, loc);
//                    cout << sol_tmp.get_objective() << " " << sol_cand.get_objective() << endl;
                    if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE ) { 
                        sol_cand = sol_tmp;
                        improved = true;
                        objectiveCpt = 0;
                    }
                    else{
                        objectiveCpt++;

                        if(objectiveCpt == TOLERANCE_CPT){
                            break;
                        }
                    }
                }
            }
            if (improved) {
                sol_best = sol_cand;
                break;
            };
        }
        if (verbose) {
            sol_best.print();
            cout << "TB loop: ";
            tock(start);
            cout << endl;
        }
    }

    checkClock();
    return sol_best;
}

Solution_cap TB::run_cap(bool verbose) {
    verbose = VERBOSE;
    
    auto sol_best = initHighestCapSolution();
    auto locations = instance->getLocations();
    bool improved = true;
    Solution_cap sol_cand;

    while (improved) {
        improved = false;
        sol_cand = sol_best;
        auto start = tick();
        auto p_locations = sol_best.get_pLocations();

        vector<uint_t> p_locations_vec;
        p_locations_vec.reserve(p_locations.size());
        for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

        for (auto loc:locations) { // First improvement over locations
            if (!p_locations.contains(loc)) {
                #pragma omp parallel for
                for (auto p_loc:p_locations_vec) { // Best improvement over p_locations
                    Solution_cap sol_tmp = sol_best;
                    if (sol_tmp.getTotalCapacity() - instance->getLocCapacity(p_loc) + instance->getLocCapacity(loc) >= instance->getTotalDemand()) {
                        sol_tmp.replaceLocation(p_loc, loc);
                        #pragma omp critical
                        if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE ) {
                            sol_cand = sol_tmp;
                            improved = true;
                        }

                    }
                }
            }

            if (improved) {
                sol_best = sol_cand;
                break;
            };
        }

        if (verbose) {
            sol_best.print();
            cout << "TB loop: ";
            tock(start);
            cout << endl;
        }
    }
    return sol_best;
}

