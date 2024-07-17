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
#include "PMP.hpp"

class PostOptimization {
public:
    // Constructor
    PostOptimization(const shared_ptr<Instance>& instance, const Solution_cap solution);
    PostOptimization(const shared_ptr<Instance>& instance, const Solution_std solution);
    // Destructor
    ~PostOptimization();


    // processSolution(const PMP& pmpSolver);

    // Methods to perform post-optimization tasks
    // void performPostOptimization();
    // void exportResults(const char* filename);
    // void saveSolution();


    // void createSelectedLocations();
    void createSelectedLocations(int num_k);
    void run();
    void run_partialOpt();

    // Getters for solution information
    Solution_std getSolution_std() const;
    Solution_cap getSolution_cap() const;
    void set_time_limit(double time);

private:
    // IloEnv env;
    // IloModel model;
    // IloCplex cplex;
    const bool is_cap;
    shared_ptr<Instance> instance;
    Solution_std solution_std;
    Solution_cap solution_cap;
    vector<uint_t> selectedLocations;


    double timelimit=0;
    // PMP pmpSolver;

    // Helper methods
    // void fetchSolutionValues();
    // void processSolution();
};

#endif // POSTOPTIMIZATION_HPP