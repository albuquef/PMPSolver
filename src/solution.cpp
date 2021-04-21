#include "solution.hpp"

Solution::Solution(Instance *instance, unordered_set<uint_t> locations): instance(instance), p_locations(std::move(locations)) {
    naiveEval();
}

void Solution::naiveEval() {
    objective = 0;
    for (auto cust:instance->getCustomers()) {
        auto loc = getClosestpLoc(cust);
        // cout << "cust: " << cust << ", loc: " << loc << ", dist: " << instance->getDist(loc, cust) << endl;
        objective += instance->getDist(loc, cust);
    }
}

uint_t Solution::getClosestpLoc(uint_t cust) {
    dist_t dist_min = numeric_limits<uint_t>::max();
    dist_t dist;
    uint_t loc_closest;
    for (auto loc:p_locations) {
        dist = instance->getDist(loc, cust);
        if (dist < dist_min) {
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
    cout << endl;
    cout << "objective: " << objective << endl << endl;
}

