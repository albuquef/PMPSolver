#ifndef POSTOPTIMIZATION_HPP
#define POSTOPTIMIZATION_HPP

#include <algorithm>
#include <iostream>
#include <chrono>
#include <cmath>
#include <ilcplex/ilocplex.h>
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "instance.hpp"
#include "solver_config.hpp"
#include "PMP.hpp"

class PostOptimization {
public:
    // Constructor
    PostOptimization(const std::shared_ptr<Instance>& instance, const Config& config, Solution_cap solution);
    PostOptimization(const std::shared_ptr<Instance>& instance, const Config& config, Solution_std solution);
    

    // Destructor
    ~PostOptimization();

    // Other methods...

    void createSelectedLocations(int num_k);
    void run(std::string Method_name);
    void run_partialOpt();

    // Getters for solution information
    Solution_std getSolution_std() const;
    Solution_cap getSolution_cap() const;
    void set_time_limit(double time);
    void setBinModel(bool isBinModel);  

private:
    const Config& config;
    const bool is_cap;
    std::shared_ptr<Instance> instance;
    Solution_std solution_std;
    Solution_cap solution_cap;
    std::vector<uint_t> selectedLocations;
    bool isBinModel=false;
    bool VERBOSE=false;
    bool add_threshold_distance=true;
    unordered_set<uint_t> tabu_locs; // exclude locations from being selected again with was in the p locations in the previous iteration and change in the current iteration
    double alpha_init=0.5;


    double timelimit=0;
    // PMP pmpSolver;

    // Helper methods
    // void fetchSolutionValues();
    // void processSolution();

    vector<uint_t> getLocationsPostOptimization(uint_t num_facilities_po, unordered_set<uint_t> p_locations, vector<uint_t>  p_locations_less_used_cap);
    void printIterationInfo(int iter, uint_t num_facilities_po, double alpha) const;
    vector<uint_t> getLessUsedLocations(const unordered_set<uint_t>& p_locations, uint_t num_facilities_added);
    vector<uint_t> getLessUsedCapacityLocations(const unordered_set<uint_t>& p_locations, uint_t num_facilities_added, uint_t p_value);
    Instance createNewInstance(const vector<uint_t>& new_locations, Solution_cap& solution_curr);
    PMP setupPMP(Instance& new_instance, const string& Method_name, bool is_feasible, Solution_cap& solution_curr);
    void evaluateSolution(Solution_cap& solution_curr, double& alpha);
    void handleNoSolutionFound(Solution_cap& solution_curr, double& alpha);
};

#endif // POSTOPTIMIZATION_HPP