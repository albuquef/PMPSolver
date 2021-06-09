//
// Created by david on 09/06/2021.
//

#ifndef LARGE_PMP_SOLUTION_CAP_H
#define LARGE_PMP_SOLUTION_CAP_H

#include <utility>
#include <unordered_set>
#include <unordered_map>
#include "globals.hpp"
#include "instance.hpp"

class Solution_cap {
private:
    unordered_set<uint_t> p_locations; // p selected locations
    dist_t objective{}; // final objective value
    shared_ptr<Instance> instance; // currently solved instance

    unordered_map<uint_t, uint_t> loc_usages; // p location -> usage from <0, capacity>
    unordered_map<uint_t, uint_t> cust_satisfactions; // customer -> satisfaction from <0, weight>
    unordered_map<uint_t, assignment> assignments; // customer -> its current assignment (p location, usage, weighted distance)
public:

    Solution_cap() = default;
    Solution_cap(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations);
    void fullEval();
    void fullCapEval();
    uint_t getClosestOpenpLoc(uint_t cust, uint_t forbidden_loc);
    const unordered_set<uint_t>& get_pLocations() const;
    void print();
    void replaceLocation(uint_t loc_old, uint_t loc_new);
    dist_t get_objective() const;
    void printAssignment();
    void exportSolution(const string& output_filename);
    vector<pair<uint_t, dist_t>> getUrgencies();

};


#endif //LARGE_PMP_SOLUTION_CAP_H
