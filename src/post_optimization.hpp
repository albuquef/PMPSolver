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


    double timelimit=0;
    // PMP pmpSolver;

    // Helper methods
    // void fetchSolutionValues();
    // void processSolution();
};

#endif // POSTOPTIMIZATION_HPP