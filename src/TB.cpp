#include "TB.hpp"

TB::TB(Instance *instance, uint_t seed):instance(instance) {
    engine.seed(seed);
    cout << "TB heuristic initialized\n";
    instance->print();
}

Solution TB::initRandomSolution() {
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

    return Solution(instance, p_locations);
}

Solution TB::run() {
    auto sol_best = initRandomSolution();
    auto locations = instance->getLocations();
    bool improved = true;
    Solution sol_tmp;
    Solution sol_cand;

    while (improved) {
        improved = false;
        sol_cand = sol_best;

        auto p_locations = sol_best.get_pLocations();
        for (auto loc:locations) { // First improvement over locations
            if (!p_locations.contains(loc)) {
                for (auto p_loc:p_locations) { // Best improvement over p_locations
                    sol_tmp = sol_best;
                    sol_tmp.replaceLocation(p_loc, loc);
                    if (sol_tmp.get_objective() < sol_cand.get_objective()) {
                        sol_cand = sol_tmp;
                        improved = true;
                    }
                }
            }
            if (improved) {
                sol_best = sol_cand;
                sol_best.print();
                break;
            };
        }
    }

    return sol_best;
}

