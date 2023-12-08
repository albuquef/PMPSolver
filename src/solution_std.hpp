#ifndef LARGE_PMP_SOLUTION_STD_HPP
#define LARGE_PMP_SOLUTION_STD_HPP

#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <vector>
#include "globals.hpp"
#include "instance.hpp"

using namespace std;

class Solution_std {
private:
    unordered_set<uint_t> p_locations; // p selected locations
    dist_t objective; // objective value
    shared_ptr<Instance> instance; // solved instance

    unordered_map<uint_t, my_pair> assignment; // customer -> p location, distance to it
public:

    Solution_std() = default;
    Solution_std(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations);
    void naiveEval();
    uint_t getClosestpLoc(uint_t cust);
    const unordered_set<uint_t>& get_pLocations() const;
    const vector<uint_t> & get_Locations() const;
    void print();
    void replaceLocation(uint_t loc_old, uint_t loc_new);
    dist_t get_objective();
    void saveAssignment(const string& output_filename,int mode);
    // Solution_std rand_swap2_Locations(Solution_std sol_current);
    // Solution_std rand_swap3_Locations(unordered_set<uint_t> p_locations);
};


#endif //LARGE_PMP_SOLUTION_STD_HPP