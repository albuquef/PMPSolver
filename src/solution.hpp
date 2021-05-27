#ifndef LARGE_PMP_SOLUTION_HPP
#define LARGE_PMP_SOLUTION_HPP

#include <utility>
#include <unordered_set>
#include <unordered_map>
#include "globals.hpp"
#include "instance.hpp"

using namespace std;

class Solution {
private:
    unordered_set<uint_t> p_locations; // p selected locations
    unordered_map<uint_t, uint_t> loc_usages; // p location -> its usage

    unordered_map<uint_t, uint_t> cust_usages; // p location -> its usage
    unordered_map<uint_t, assignment> assignments; // customer -> its current assignment
    dist_t objective; // objective value
public:
    shared_ptr<Instance> instance; // currently solved instance

    Solution() = default;
    Solution(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations);
    void fullEval();
    void fullCapEval();
    uint_t getClosestpLoc(uint_t cust);
    const unordered_set<uint_t>& get_pLocations() const;
    void print();
    void replaceLocation(uint_t loc_old, uint_t loc_new);
    dist_t get_objective();
    void printAssignment();
    void exportSolution(const string& output_filename);

};


#endif //LARGE_PMP_SOLUTION_HPP
