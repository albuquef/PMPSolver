#ifndef PMPSOLVER_SOLUTION_CAP_H
#define PMPSOLVER_SOLUTION_CAP_H

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
    bool isFeasible=false;
    bool cover_mode=false;
    bool cover_mode_n2=false;
    double UpperBound=0;
    dist_t max_dist=0;
    dist_t min_dist=numeric_limits<dist_t>::max();;
    dist_t avg_dist=0;
    dist_t std_dev_dist=0;
    uint max_num_assignments=0;

    dist_t Best_Bound=0;

public:

    Solution_cap() = default;
    // Solution_cap(shared_ptr<Instance> instance);
    Solution_cap(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations, const char* typeEVAL="GAPrelax", bool cover_mode=false);
    Solution_cap(shared_ptr<Instance> instance,
                 unordered_set<uint_t> p_locations,
                 unordered_map<uint_t, dist_t> loc_usages, 
                 unordered_map<uint_t, dist_t> cust_satisfactions, 
                 unordered_map<uint_t, assignment> assignments);
    void fullCapEval();
    void GAP_eval();
    void naiveEval();
    void objEval();
    uint_t getClosestpLoc(uint_t cust);
    uint_t getClosestOpenpLoc(uint_t cust, uint_t forbidden_loc);
    const unordered_set<uint_t>& get_pLocations() const;
    void print();
    void replaceLocation(uint_t loc_old, uint_t loc_new, const char* typeEval="GAPrelax");
    dist_t get_objective() const;
    vector<pair<uint_t, dist_t>> getUrgencies();
    uint_t getTotalCapacity();
    void setLocUsage(uint_t loc, dist_t usage);
    void setCustSatisfaction(uint_t cust, dist_t satisfaction);
    void setAssigment(uint_t cust, assignment assigment);
    unordered_map<uint_t, dist_t> getLocUsages();
    unordered_map<uint_t, dist_t> getCustSatisfactions();
    unordered_map<uint_t, assignment> getAssignments();
    void saveAssignment(string output_filename,string Method, double timeFinal);
    void saveResults(string output_filename, double timeFinal, int numIter,string Method, string Method_sp="null", string Method_fp="null");
    void setFeasibility(bool feasible);
    bool getFeasibility();
    bool isSolutionFeasible();  
    bool isCoverMode();
    bool isCoverModeN2();
    void setCoverMode(bool cover_mode);
    void setCoverMode_n2(bool cover_mode_n2);
    void add_UpperBound(double UB); 
    
    void statsDistances();
    dist_t getMaxDist();
    dist_t getMinDist();
    dist_t getAvgDist();
    dist_t getStdDevDist();
    uint_t getMaxNumberAssignments();
    uint_t calculateMaxNumberAssignments();
    void printStatsDistances();

    void setBestBound(dist_t Best_Bound);
    dist_t getBestBound();

    bool capacitiesAssigmentRespected(const shared_ptr<Instance>& instance_total);
};


#endif //PMPSOLVER_SOLUTION_CAP_H
