#ifndef PMPSOLVER_SOLVER_MANAGER_HPP
#define PMPSOLVER_SOLVER_MANAGER_HPP

#include <chrono>
#include <iostream>
#include <memory>
#include "globals.hpp"
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"
#include "utils.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "solution_map.hpp"
#include "PMP.hpp"
#include "VNS.hpp"
#include "post_optimization.hpp"

using namespace std;    
using namespace chrono;

class SolverManager {
public:
    SolverManager(const Config& config, const Instance& instance);

    void solveProblem();

    Solution_std methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
    Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);

private:
    const Config& config;
    const Instance& instance;

    void solveCPMP();
    void solveRSSV();

    template <typename SolverFunc>
    void solveFinalProblem(const std::shared_ptr<Instance>& filtered_instance, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time_total);

    template <typename SolutionType>
    void runPostOptimization(SolutionType& solution, double elapsed_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time_total);

    template <typename SolutionType>
    void printSolution(const SolutionType& solution, double elapsed_time) const;

    template <typename SolutionType>
    void saveSolution(const SolutionType& solution, double elapsed_time) const;
};

#endif // PMPSOLVER_SOLVER_MANAGER_HPP