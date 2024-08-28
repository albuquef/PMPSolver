#include "solver_manager.hpp"

SolverManager::SolverManager(const Config& config, const Instance& instance)
    : config(config), instance(instance) {

        seed = config.seed;
    }


bool SolverManager::isPMPMethod(const string& method) {
    return method == "EXACT_PMP" || method == "TB_PMP" || method == "VNS_PMP";
}

bool SolverManager::isCPMPMethod(const string& method) {
    return method == "EXACT_CPMP" || method == "EXACT_CPMP_BIN" || method == "TB_CPMP" || 
           method == "VNS_CPMP" || method == "GAPrelax" || method == "GAP";
}

// template <typename SolutionType>
// void SolverManager::saveOutputs(const SolutionType& solution, double elapsed_time, string add_INFO="") {
//     // cout << "Final solution:\n";
//     // solution.print();
//     // solution.statsDistances();
//     // solution.printStatsDistances();
//     // cout << "Final total elapsed time: " << elapsed_time << "s\n";
//     if (config.Method == "RSSV") {
//         string method_final = config.Method + "_" + config.Method_RSSV_fp + add_INFO;
//         solution.saveAssignment(config.output_filename, method_final, elapsed_time);
//         solution.saveResults(config.output_filename, elapsed_time, 0, method_final);
//     }else{
//         string method_final = config.Method + add_INFO;
//         solution.saveAssignment(config.output_filename, method_final, elapsed_time);
//         solution.saveResults(config.output_filename, elapsed_time, 0, method_final);
//     }
// }


// void SolverManager::solveProblem(const Instance& instance, const Config& config, int seed) {
void SolverManager::solveProblem() {
    cout << "-------------------------------------------------\n";


    auto start_time = high_resolution_clock::now();
    shared_ptr<Instance> shared_instance = make_shared<Instance>(instance);

    if (config.Method == "RSSV") {
        runRSSVHeuristic(shared_instance, config, seed, start_time);
    } else {
        if (isPMPMethod(config.Method)) {
            runPMPMethod(shared_instance, config, start_time);
        } else if (isCPMPMethod(config.Method)) {
            auto solution = runCPMPMethod(shared_instance, config, start_time);
            cout << "Post Optimization: " << config.Method_PostOpt  << endl;
            if (config.Method_PostOpt != "null" && config.Method_PostOpt != "") {
                runPostOptimization(solution, config, external_time, start_time);
            }
        } else {
            cerr << "[ERROR] Method not found" << endl;
            exit(1);
        }
    }

    cout << "Finish solving problem\n";
}


Solution_std SolverManager::runPMPMethod(const shared_ptr<Instance>& instance, const Config& config, const time_point<high_resolution_clock>& start_time) {
    Solution_std solution = methods_PMP(instance, config, external_time);
    auto elapsed_time = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();

    cout << "\nFinal solution PMP:\n";
    solution.print();
    solution.statsDistances();
    cout << "Final total elapsed time: " << elapsed_time << "s\n";
    solution.saveAssignment(config.output_filename, config.Method, elapsed_time);
    solution.saveResults(config.output_filename, elapsed_time, 0, config.Method);  


    return solution;
}


Solution_cap SolverManager::runCPMPMethod(const shared_ptr<Instance>& instance, const Config& config, const time_point<high_resolution_clock>& start_time) {
    Solution_cap solution = methods_CPMP(instance, config, external_time);
    auto elapsed_time_cpmp = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();

    cout << "\nFinal solution CPMP:\n";
    solution.print();
    solution.statsDistances();
    solution.printStatsDistances();
    cout << "Final total elapsed time: " << elapsed_time_cpmp << "s\n"; 

    auto elapsed_time = elapsed_time_cpmp;
    string add_INFO = "";
    if (config.Method == "RSSV") {
        string method_final = config.Method + "_" + config.Method_RSSV_fp + add_INFO;
        solution.saveAssignment(config.output_filename, method_final, elapsed_time);
        solution.saveResults(config.output_filename, elapsed_time, 0, method_final);
    }else{
        string method_final = config.Method + add_INFO;
        solution.saveAssignment(config.output_filename, method_final, elapsed_time);
        solution.saveResults(config.output_filename, elapsed_time, 0, method_final);
    }
    
    external_time = elapsed_time_cpmp;

    // runPostOptimization(solution, config, elapsed_time_cpmp, start_time);
    return solution;
}

void SolverManager::runPostOptimization(Solution_cap& solution, const Config& config, double elapsed_time_outside, const time_point<high_resolution_clock>& start_time) {
    cout << "\n\n-------------------------------------------------\n";
    cout << "Post Optimization\n";
    cout << "-------------------------------------------------\n";
    cout << "method: " << config.Method_PostOpt << "\n";
    cout << "-------------------------------------------------\n";
    auto time_left = config.CLOCK_LIMIT - external_time;
    cout << "Time for post-optimization: " << time_left << "s\n";
    const shared_ptr<Instance>& instance_postopt = make_shared<Instance>(instance);
    PostOptimization postOptimization(instance_postopt, config, solution);
    postOptimization.set_time_limit(time_left);
    postOptimization.run(config.Method + "_POSTOPT");
    auto elapsed_time_postopt = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();
    solution = postOptimization.getSolution_cap();
    cout << "\n\n\n";
    cout << "Finishing post-optimization" << endl;
    solution.print(); 
    
    cout << "Solution Stats" << endl;
    solution.statsDistances();
    solution.printStatsDistances();

    auto elapsed_time = elapsed_time_postopt;
    string add_INFO = "_POSTOPT";
    if (config.Method == "RSSV") {
        string method_final = config.Method + "_" + config.Method_RSSV_fp + add_INFO;
        solution.saveAssignment(config.output_filename, method_final, elapsed_time);
        solution.saveResults(config.output_filename, elapsed_time, 0, method_final);
    }else{
        string method_final = config.Method + add_INFO;
        solution.saveAssignment(config.output_filename, method_final, elapsed_time);
        solution.saveResults(config.output_filename, elapsed_time, 0, method_final);
    } 

    cout << "Final total elapsed time (post opt): " << elapsed_time_postopt << "s\n";

}

void SolverManager::runRSSVHeuristic(const shared_ptr<Instance>& instance, const Config& config, int seed, const time_point<high_resolution_clock>& start_time) {
    
    cout << "\n\n-------------------------------------------------\n";
    cout << "RSSV heuristic \n";
    cout << "-------------------------------------------------\n";
    cout << "method subproblems: " << config.Method_RSSV_sp << "\n";
    cout << "method final problem: " << config.Method_RSSV_fp << "\n";
    cout << "-------------------------------------------------\n";

    RSSV metaheuristic(instance, seed, SUB_PMP_SIZE);
    setRSSVParameters(metaheuristic, config);

    shared_ptr<Instance> filtered_instance = runRSSVSubproblem(metaheuristic, config);
    setFilterInstanceParameters(filtered_instance, config);
    external_time = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();

    cout << "\n-------------------------------------------------\n";
    cout << "Final Problem RSSV heuristic"<< "\n";

    // return to main function
    if (isPMPMethod(config.Method_RSSV_fp)) {
        auto solution = runPMPMethod(filtered_instance, config, start_time);
    } else if (isCPMPMethod(config.Method_RSSV_fp)) {
        auto solution = runCPMPMethod(filtered_instance, config, start_time);
        cout << "Post Optimization: " << config.Method_PostOpt  << endl;
        if (config.Method_PostOpt != "null" || config.Method_PostOpt != "") {
            runPostOptimization(solution, config, external_time, start_time);
        }
    } else {
        cerr << "[ERROR] Method subproblem RSSV not found" << endl;
        exit(1);
    }
}

shared_ptr<Instance> SolverManager::runRSSVSubproblem(RSSV& metaheuristic, const Config& config) {
    if (isPMPMethod(config.Method_RSSV_sp)) {
        return metaheuristic.run(THREAD_NUMBER, config.Method_RSSV_sp);
    } else if (isCPMPMethod(config.Method_RSSV_sp)) {
        return metaheuristic.run_CAP(THREAD_NUMBER, config.Method_RSSV_sp);
    } else {
        cerr << "[ERROR] Method subproblem RSSV not found" << endl;
        exit(1);
    }
}


void SolverManager::setFilterInstanceParameters(shared_ptr<Instance>& filtered_instance, const Config& config) {
    filtered_instance->setCoverModel(config.cover_mode);
    filtered_instance->setCoverModel_n2(config.cover_mode_n2);
    filtered_instance->set_isWeightedObjFunc(config.IsWeighted_ObjFunc);
    cout << "Final instance parameters:\n";
    filtered_instance->print();
}


void SolverManager::setRSSVParameters(RSSV& metaheuristic, const Config& config) {
    metaheuristic.setCoverMode(config.cover_mode);
    metaheuristic.setCoverMode_n2(config.cover_mode_n2);
    metaheuristic.setMAX_ITE_SUBPROBLEMS(config.MAX_ITE_SUBPROB_RSSV);
    metaheuristic.setTIME_LIMIT_SUBPROBLEMS(config.CLOCK_LIMIT_SUBPROB_RSSV);
    if (config.fixed_threshold_distance <= 0) metaheuristic.setAddThresholdDist(config.add_threshold_distance_rssv);
    CLOCK_THREADED = true;
}


void SolverManager::setExactMethodParams(PMP& pmp, const Config& config, const shared_ptr<Instance>& instance, double external_time) {
    pmp.setGenerateReports(config.add_generate_reports);
    pmp.setAddBreakCallback(config.add_break_callback);
    pmp.setCoverModel(config.cover_mode, instance->getTypeSubarea());
    pmp.setCoverModel_n2(config.cover_mode_n2, instance->getTypeSubarea_n2());
    pmp.setTimeLimit(config.CLOCK_LIMIT_CPLEX - external_time);
    if (config.CLOCK_LIMIT_CPLEX == 0) {pmp.setTimeLimit(0);} // no time limit

    if(config.Method == "RSSV") {handleInitialSolution(instance, pmp, config, config.Method + "_" + config.Method_RSSV_fp);}

}
template<typename Heuristic>
void SolverManager::setHeuristicParams(Heuristic& heuristic, string Method, const Config& config, const Solution_MAP& solution_map, double external_time){
    
    heuristic.setSolutionMap(solution_map);
    heuristic.setGenerateReports(true);
    heuristic.setMethod(Method);
    heuristic.setCoverMode(config.cover_mode);
    heuristic.setCoverMode_n2(config.cover_mode_n2);
    heuristic.setTimeLimit(config.CLOCK_LIMIT - external_time);
}

void SolverManager::handleInitialSolution(const shared_ptr<Instance>& instance, PMP& pmp, const Config& config, const string& method) {
    bool add_InitialSolution_RSSV = false;
    if (add_InitialSolution_RSSV) {
        auto vet_locs = instance->getVotedLocs();
        unordered_set<uint_t> init_p(vet_locs.begin(), vet_locs.begin() + instance->get_p());
        Solution_cap init_sol(instance, init_p, method == "RSSV_EXACT_CPMP" ? "GAPrelax" : "GAP");
        if (init_sol.getFeasibility()) {
            pmp.setMIPStartSolution(init_sol);
            init_sol.print();
        } else {
            cout << "Initial solution not feasible\n";
        }
    }
}


void printHeader(const string& method_name) {
    cout << "-------------------------------------------------\n";
    cout << "    " << method_name << "\n";
    cout << "-------------------------------------------------\n";
}

Solution_std SolverManager::methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time) {
    Solution_std solution;
    cout << "-------------------------------------------------\n";
    string Method = config.Method;
    if (Method == "RSSV") {
        Method = config.Method + "_" + config.Method_RSSV_fp;
    }
    
    printHeader(Method);

    if (Method == "EXACT_PMP" || Method == "RSSV_EXACT_PMP")  {
        PMP pmp(instance, "PMP");
        setExactMethodParams(pmp, config, instance, external_time);
        pmp.run(config.Method);
        pmp.saveVars(config.output_filename, Method);
        pmp.saveResults(config.output_filename, Method);
        solution = pmp.getSolution_std();
    } else if (Method == "TB_PMP" || Method == "RSSV_TB_PMP") {
        TB heuristic(instance, config.seed);
        setHeuristicParams(heuristic, Method, config, Solution_MAP(instance), external_time);
        solution = heuristic.run(true, UB_MAX_ITER);
    } else if (Method == "VNS_PMP" || Method == "RSSV_VNS_PMP") {
        VNS heuristic(instance, config.seed);
        setHeuristicParams(heuristic, Method, config, Solution_MAP(instance), external_time);
        solution = heuristic.runVNS_std(true, UB_MAX_ITER);
    } else {
        cout << "[ERROR] Method PMP not found" << endl;
        exit(1);
    }
    return solution;
}
Solution_cap SolverManager::methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time) {
    Solution_cap solution;
    Solution_MAP solution_map(instance);

    string Method = config.Method;
    if (Method == "RSSV") {
        Method = config.Method + "_" + config.Method_RSSV_fp;
    }

    printHeader(Method);


    if (Method == "EXACT_CPMP" || Method == "RSSV_EXACT_CPMP") {
        PMP pmp(instance, "CPMP");
        setExactMethodParams(pmp, config, instance, external_time);
        pmp.run(Method);
        pmp.saveVars(config.output_filename, Method);
        pmp.saveResults(config.output_filename, Method);
        solution = pmp.getSolution_cap();
    } else if (Method == "EXACT_CPMP_BIN" || Method == "RSSV_EXACT_CPMP_BIN") {
        PMP pmp(instance, "CPMP", true);
        setExactMethodParams(pmp, config, instance, external_time);
        pmp.run(Method);
        pmp.saveVars(config.output_filename, Method);
        pmp.saveResults(config.output_filename, Method);
        solution = pmp.getSolution_cap();
    } else if (Method == "TB_CPMP" || Method == "RSSV_TB_CPMP") {
        TB heuristic(instance, config.seed);
        setHeuristicParams(heuristic, Method, config, solution_map, external_time);
        solution = heuristic.run_cap(true, UB_MAX_ITER);
    } else if (Method == "VNS_CPMP" || Method == "RSSV_VNS_CPMP") {
        VNS heuristic(instance, config.seed);
        setHeuristicParams(heuristic, Method, config, solution_map, external_time);
        solution = heuristic.runVNS_cap(Method, true, UB_MAX_ITER);

    } else if (Method == "GAPrelax" || Method == "GAP") {
        TB heuristic(instance, config.seed);
        setHeuristicParams(heuristic, Method, config, solution_map, external_time);
        auto solution = heuristic.fixedCapSolution(Method);
        cout << "Final solution:\n";
        solution.print();
        exit(1);

    } else {
        cout << "[ERROR] Method CPMP not found" << endl;
        exit(1);
    }


    auto sol_best = solution;
    sol_best.setCoverMode(config.cover_mode);
    sol_best.setCoverMode_n2(config.cover_mode_n2);

    return sol_best;
}