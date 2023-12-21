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
// #include "PMP.hpp"

class Solution_cap {
private:
    unordered_set<uint_t> p_locations; // p selected locations
    dist_t objective{}; // final objective value
    shared_ptr<Instance> instance; // currently solved instance
    const char* typeEval;


    unordered_map<uint_t, dist_t> loc_usages; // p location -> usage from <0, capacity>
    unordered_map<uint_t, dist_t> cust_satisfactions; // customer -> satisfaction from <0, weight>
    unordered_map<uint_t, assignment> assignments; // customer -> its current assignment (p location, usage, weighted distance)
    void setSolution(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations
                    ,unordered_map<uint_t, dist_t>  loc_usages,unordered_map<uint_t, dist_t> cust_satisfactions
                    ,unordered_map<uint_t, assignment> assignments, dist_t objective);
public:

    Solution_cap() = default;
    // Solution_cap(shared_ptr<Instance> instance);
    Solution_cap(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations, const char* typeEVAL="GAPrelax");
    Solution_cap(shared_ptr<Instance> instance,
                 unordered_set<uint_t> p_locations,
                 unordered_map<uint_t, dist_t> loc_usages, 
                 unordered_map<uint_t, dist_t> cust_satisfactions, 
                 unordered_map<uint_t, assignment> assignments,
                 dist_t objective);
    void fullCapEval();
    void GAP_eval();
    void naiveEval();
    uint_t getClosestpLoc(uint_t cust);
    uint_t getClosestOpenpLoc(uint_t cust, uint_t forbidden_loc);
    const unordered_set<uint_t>& get_pLocations() const;
    void print();
    void replaceLocation(uint_t loc_old, uint_t loc_new, const char* typeEval="GAPrelax");
    dist_t get_objective() const;
    void saveAssignment(string output_filename,int mode);
    vector<pair<uint_t, dist_t>> getUrgencies();
    uint_t getTotalCapacity();
    void setLocUsage(uint_t loc, dist_t usage);
    void setCustSatisfaction(uint_t cust, dist_t satisfaction);
    void setAssigment(uint_t cust, assignment assigment);
    unordered_map<uint_t, dist_t> getLocUsages();
    unordered_map<uint_t, dist_t> getCustSatisfactions();
    unordered_map<uint_t, assignment> getAssignments();
    void objEval();

};


#endif //LARGE_PMP_SOLUTION_CAP_H
