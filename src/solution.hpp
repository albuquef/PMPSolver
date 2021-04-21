#ifndef LARGE_PMP_SOLUTION_HPP
#define LARGE_PMP_SOLUTION_HPP

#include <utility>
#include <unordered_set>
#include "types.hpp"
#include "instance.hpp"

using namespace std;

class Solution {
private:
    unordered_set<uint_t> p_locations;
    dist_t objective;
public:
    Instance *instance;

    Solution() = default;
    Solution(Instance *instance, unordered_set<uint_t> locations);
    void naiveEval();
    uint_t getClosestpLoc(uint_t cust);
    const unordered_set<uint_t>& get_pLocations() const;
    void print();
    void replaceLocation(uint_t loc_old, uint_t loc_new);
    dist_t get_objective();

};


#endif //LARGE_PMP_SOLUTION_HPP
