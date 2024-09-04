#ifndef PMPSOLVER_SOLUTION_STD_HPP
#define PMPSOLVER_SOLUTION_STD_HPP

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

    bool cover_mode = false;
    bool cover_mode_n2 = false;
    bool is_weighted_obj_func;
    double UpperBound=0;
    dist_t max_dist=0;
    dist_t min_dist=numeric_limits<dist_t>::max();;
    dist_t avg_dist=0;
    dist_t std_dev_dist=0; 
    uint max_num_assignments=0;
public:

    Solution_std() = default;
    Solution_std(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations);
    // Solution_std(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations,unordered_map<uint_t, my_pair> assignment);
    void naiveEval();
    uint_t getClosestpLoc(uint_t cust);
    const unordered_set<uint_t>& get_pLocations() const;
    const vector<uint_t> & get_Locations() const;
    void print();
    void replaceLocation(uint_t loc_old, uint_t loc_new);
    dist_t get_objective();
    void saveAssignment(string output_filename,string Method, double timeFinal);
    void saveResults(string output_filename, double timeFinal, int numIter,string Method, string Method_sp="null", string Method_fp="null");

    bool isSolutionFeasible();

    void setCoverMode(bool mode) {
        cover_mode = mode;
    }
    void setCoverMode_n2(bool mode) {
        cover_mode_n2 = mode;
    }
    void statsDistances();
    dist_t getMaxDist();
    dist_t getMinDist();
    dist_t getAvgDist();
    dist_t getStdDevDist();

    uint_t getMaxNumberAssignments();
    uint_t calculateMaxNumberAssignments();
};


#endif //PMPSOLVER_SOLUTION_STD_HPP