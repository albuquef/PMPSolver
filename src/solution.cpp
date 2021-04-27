#include <iomanip>
#include <cassert>
#include "solution.hpp"

Solution::Solution(Instance *instance, unordered_set<uint_t> p_locations) {
    this->instance = instance;
    this->p_locations = p_locations;
    naiveEval();
}

void Solution::naiveEval() {
//    assert(p_locations.size() == instance->get_p());
    objective = 0;
    for (auto cust:instance->getCustomers()) {
        auto loc = getClosestpLoc(cust);
        objective += instance->getDist(loc, cust);
    }
}

uint_t Solution::getClosestpLoc(uint_t cust) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    dist_t dist;
    uint_t loc_closest;
    for (auto loc:p_locations) {
        dist = instance->getDist(loc, cust);
        if (dist <= dist_min) {
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
    cout << setprecision(15) << "objective: " << objective << endl << endl;
}

const unordered_set<uint_t> &Solution::get_pLocations() const {
    return this->p_locations;
}

void Solution::replaceLocation(uint_t loc_old, uint_t loc_new) {
    p_locations.erase(loc_old);
    p_locations.insert(loc_new);
    naiveEval();
}

dist_t Solution::get_objective() {
    return objective;
}



