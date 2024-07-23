#ifndef PMPSOLVER_SOLVER_MANAGER_HPP
#define PMPSOLVER_SOLVER_MANAGER_HPP

#include <chrono>
#include <iostream>
#include <memory>
#include "globals.hpp"
#include "instance.hpp"
#include "solver_config.hpp"
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

    // void solveProblem(const Instance& instance, const Config& config, int seed);
    void solveProblem();
    Solution_std methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
    Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);


private:
    const Config& config;
    const Instance& instance;
    double external_time = 0;
    int seed = 0;
    // void solveCPMP();
    // void solveRSSV();
    template<typename Heuristic>
    void setHeuristicParams(Heuristic& heuristic, string Method, const Config& config, const Solution_MAP& solution_map, double external_time);
    void setExactMethodParams(PMP& pmp, const Config& config, const shared_ptr<Instance>& instance, double external_time);

    bool isPMPMethod(const string& method);
    bool isCPMPMethod(const string& method);
    void handleInitialSolution(const shared_ptr<Instance>& instance, PMP& pmp, const Config& config, const string& method);


    Solution_std runPMPMethod(const shared_ptr<Instance>& instance, const Config& config, const time_point<high_resolution_clock>& start_time);
    Solution_cap runCPMPMethod(const shared_ptr<Instance>& instance, const Config& config, const time_point<high_resolution_clock>& start_time);
    
    // template<typename SolutionType, typename MethodType>
    // SolutionType solveMethod(const shared_ptr<Instance>& instance, const Config& config, double external_time, MethodType methodHandler)

    void runRSSVHeuristic(const shared_ptr<Instance>& instance, const Config& config, int seed, const time_point<high_resolution_clock>& start_time_total);
    shared_ptr<Instance> runRSSVSubproblem(RSSV& metaheuristic, const Config& config);
    void setFilterInstanceParameters(shared_ptr<Instance>& filtered_instance, const Config& config);
    void setRSSVParameters(RSSV& metaheuristic, const Config& config);

    void runPostOptimization(Solution_cap& solution, const Config& config, double elapsed_time_total, const time_point<high_resolution_clock>& start_time);
    // template <typename SolutionType>
    // void saveOutputs(const SolutionType& solution, double elapesed_time, string add_INFO="");
    

};

#endif // PMPSOLVER_SOLVER_MANAGER_HPP