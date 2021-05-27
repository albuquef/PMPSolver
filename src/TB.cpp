#include "TB.hpp"

#include <utility>

TB::TB(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
//    cout << "TB heuristic initialized\n";
//    instance->print();
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
    Solution sol(instance, p_locations);
    return sol;
}

//Solution TB::run(bool verbose) {
//    auto sol_best = initRandomSolution();
//    auto locations = instance->getLocations();
//    bool improved = true;
//    Solution sol_tmp;
//    Solution sol_cand;
//
//    while (improved) {
//        improved = false;
//        sol_cand = sol_best;
//        auto start = tick();
//        auto p_locations = sol_best.get_pLocations();
//        for (auto loc:locations) { // First improvement over locations
//            if (!p_locations.contains(loc)) {
//                for (auto p_loc:p_locations) { // Best improvement over p_locations
//                    sol_tmp = sol_best;
//                    sol_tmp.replaceLocation(p_loc, loc);
////                    cout << sol_tmp.get_objective() << " " << sol_cand.get_objective() << endl;
//                    if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE ) {
//                        sol_cand = sol_tmp;
//                        improved = true;
//                    }
//                }
//            }
//            if (improved) {
//                sol_best = sol_cand;
//                break;
//            };
//        }
//        if (verbose) {
//            sol_best.print();
//            cout << "TB loop: ";
//            tock(start);
//            cout << endl;
//        }
//    }
//    return sol_best;
//}

