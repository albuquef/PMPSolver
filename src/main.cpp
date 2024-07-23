#include <set>
#include <cstring>
#include <string>
#include <chrono> // for time-related functions
using namespace std;
using namespace chrono;


#include "globals.hpp"
#include "solver_config.hpp" // #include "config_parser.cpp"
#include "solver_manager.hpp"
#include "instance.hpp"
#include "RSSV.hpp"
#include "TB.hpp"
#include "TBPercentage.hpp"
#include "utils.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "solution_map.hpp"
#include "PMP.hpp"
#include "VNS.hpp"
#include "post_optimization.hpp"

Instance setupInstance(const Config& config) {
    cout << "Loading instance...\n";
    
    // Load instance
    Instance instance(config.dist_matrix_filename, 
                      config.labeled_weights_filename, 
                      config.capacities_filename, 
                      config.p, ' ', 
                      config.TypeService, 
                      config.cust_max_id, 
                      config.loc_max_id);

    if (!config.coverages_filename.empty() && config.cover_mode) {
        instance.ReadCoverages(config.coverages_filename, config.TypeSubarea, ' ');
        instance.setCoverModel(true);
    }

    if (!config.coverages_filename_n2.empty() && config.cover_mode_n2) {
        instance.ReadCoverages_n2(config.coverages_filename_n2, config.TypeSubarea_n2, ' ');
        instance.setCoverModel_n2(true);
    }

    cout << "[INFO] Instance loaded\n";
    instance.set_isWeightedObjFunc(config.IsWeighted_ObjFunc);
    instance.print();

    return instance;
}

Solution_MAP solution_map;
Solution_std methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time);
void solveProblem(const Instance& instance, const Config& config, int seed);


int main(int argc, char *argv[]) {

    Config config;
    set<const char*> configOverride;
    try {
        parseArguments(argc, argv, config);
        setupConfig(config, config.configOverride);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    // Check required parameters before proceeding
    checkRequiredParameters(config);
    setThreadNumber(config.threads_cnt);

    Instance instance = setupInstance(config);

    // filter instance
    // Instance instance_original = setupInstance(config); // Instance instance = instance_original.filterInstance(TypeService);

    auto start = tick();
    SolverManager solverManager(config, instance);
    solverManager.solveProblem();
    tock(start);



    // auto start = tick();
    // solveProblem(instance, config, config.seed);
    // cout << endl;
    // tock(start);

    return 0;
}


void solveProblem(const Instance& instance, const Config& config, int seed) {
    cout << "-------------------------------------------------\n";
    
    if (config.Method == "EXACT_PMP" || config.Method == "TB_PMP" || config.Method == "VNS_PMP") {
        auto start_time = high_resolution_clock::now(); // only clock can give CPU time
        Solution_std solution = methods_PMP(make_shared<Instance>(instance), config, 0);
        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
        
        cout << "\nFinal solution std:\n";
        solution.print();
        solution.statsDistances();
        // solution.printStatsDistances(); // not implemented yet
        cout << "Final total elapsed time: " << elapsed_time << "s\n";
        solution.saveAssignment(config.output_filename, config.Method, elapsed_time);
        solution.saveResults(config.output_filename, elapsed_time, 0, config.Method);   
    } 
    else if (config.Method == "EXACT_CPMP" || config.Method == "EXACT_CPMP_BIN" || config.Method == "TB_CPMP" || 
             config.Method == "VNS_CPMP" || config.Method == "GAPrelax" || config.Method == "GAP") {
        
        auto start_time = high_resolution_clock::now();
        Solution_cap solution = methods_CPMP(make_shared<Instance>(instance), config, 0);
        auto current_time = high_resolution_clock::now();
        auto elapsed_time_total = duration_cast<seconds>(current_time - start_time).count();

        cout << "\nFinal solution:\n";
        solution.print();
        solution.statsDistances();
        solution.printStatsDistances();
        cout << "Final total elapsed time: " << elapsed_time_total << "s\n";  
        solution.saveAssignment(config.output_filename, config.Method, elapsed_time_total);
        solution.saveResults(config.output_filename, elapsed_time_total, 0, config.Method);


        cout << "\n\n";
        cout << "-------------------------------------------------\n";
        cout << "Post Optimization\n";
        cout << "-------------------------------------------------\n";
        auto time_left = config.CLOCK_LIMIT - elapsed_time_total;
        cout << "Time for post-optimization: " << time_left << "s\n";
        PostOptimization postOptimization(make_shared<Instance>(instance),config, solution);
        postOptimization.set_time_limit(time_left);
        postOptimization.run(config.Method + "_POSTOPT");
        auto elapsed_time_postopt = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();
        solution = postOptimization.getSolution_cap();
        solution.saveAssignment(config.output_filename, config.Method + "_POSTOPT", elapsed_time_postopt);
        solution.saveResults(config.output_filename, elapsed_time_postopt, 0, config.Method + "_POSTOPT");




    } 
    else if (config.Method == "RSSV") {
        cout << "RSSV heuristic \n";
        cout << "-------------------------------------------------\n";
        
        RSSV metaheuristic(make_shared<Instance>(instance), seed, SUB_PMP_SIZE);
        metaheuristic.setCoverMode(config.cover_mode);
        metaheuristic.setCoverMode_n2(config.cover_mode_n2);
        metaheuristic.setMAX_ITE_SUBPROBLEMS(config.MAX_ITE_SUBPROB_RSSV);
        metaheuristic.setTIME_LIMIT_SUBPROBLEMS(config.CLOCK_LIMIT_SUBPROB_RSSV);
        metaheuristic.setAddThresholdDist(config.add_threshold_distance_rssv);
        CLOCK_THREADED = true;
        auto start_time_total = high_resolution_clock::now();

        shared_ptr<Instance> filtered_instance;
        if (config.Method_RSSV_sp == "EXACT_PMP" || config.Method_RSSV_sp == "TB_PMP" || config.Method_RSSV_sp == "VNS_PMP") {
            filtered_instance = metaheuristic.run(THREAD_NUMBER, config.Method_RSSV_sp);
        } 
        else if (config.Method_RSSV_sp == "EXACT_CPMP" || config.Method_RSSV_sp == "EXACT_CPMP_BIN" || 
                 config.Method_RSSV_sp == "TB_CPMP" || config.Method_RSSV_sp == "VNS_CPMP") {
            filtered_instance = metaheuristic.run_CAP(THREAD_NUMBER, config.Method_RSSV_sp);
        } 
        else {
            cerr << "[ERROR] Method subproblem RSSV not found" << endl;
            exit(1);
        }

        filtered_instance->setCoverModel(config.cover_mode);
        filtered_instance->setCoverModel_n2(config.cover_mode_n2);
        filtered_instance->set_isWeightedObjFunc(config.IsWeighted_ObjFunc);
        
        cout << "Final instance parameters:\n";
        filtered_instance->print();

        cout << "-------------------------------------------------\n";
        cout << "Final Problem RSSV heuristic \n";
        cout << "-------------------------------------------------\n";
        
        if (config.Method_RSSV_fp == "EXACT_PMP" || config.Method_RSSV_fp == "TB_PMP" || config.Method_RSSV_fp == "VNS_PMP") {
            auto start_time = high_resolution_clock::now();
            Solution_std solution = methods_PMP(filtered_instance, config,
                                    duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();

            cout << "\nFinal solution std:\n";
            solution.print();
            cout << "Final problem elapsed time: " << elapsed_time << "s\n";
            cout << "Final total elapsed time: " << duration_cast<seconds>(current_time - start_time_total).count() << "s\n";
            solution.saveAssignment(config.output_filename, config.Method, elapsed_time);
            solution.saveResults(config.output_filename, elapsed_time, 0, config.Method); 
        } 
        else if (config.Method_RSSV_fp == "EXACT_CPMP" || config.Method_RSSV_fp == "EXACT_CPMP_BIN" || 
                 config.Method_RSSV_fp == "TB_CPMP" || config.Method_RSSV_fp == "VNS_CPMP") {
            auto start_time = high_resolution_clock::now();        
            Solution_cap solution = methods_CPMP(filtered_instance, config,
                                                duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            
            if (config.Method_RSSV_fp == "EXACT_CPMP" || config.Method_RSSV_fp == "EXACT_CPMP_BIN") {
                solution.objEval();
            }

            cout << "\nFinal solution:\n";
            solution.print();
            cout << "Final elapsed time: " << elapsed_time << "s\n";
            auto elapsed_time_rssv = duration_cast<seconds>(current_time - start_time_total).count();
            cout << "Final total elapsed time RSSV: " << elapsed_time_rssv << "s\n";
            solution.saveAssignment(config.output_filename, "RSSV_" + config.Method_RSSV_fp, elapsed_time);
            solution.saveResults(config.output_filename, elapsed_time, 0, config.Method, config.Method_RSSV_sp, config.Method_RSSV_fp);
            cout << "\n\n\n";

            cout << "Solution Stats" << endl;
            solution.statsDistances();
            solution.printStatsDistances();

            cout << "\n\n";
            cout << "-------------------------------------------------\n";
            cout << "Post Optimization\n";
            cout << "-------------------------------------------------\n";
            auto time_left = config.CLOCK_LIMIT - elapsed_time_rssv;
            cout << "Time for post-optimization: " << time_left << "s\n";
            PostOptimization postOptimization(make_shared<Instance>(instance), config, solution);
            postOptimization.set_time_limit(time_left);
            postOptimization.run("RSSV_" + config.Method_RSSV_fp + "_POSTOPT");
            auto elapsed_time_postopt = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();
            solution = postOptimization.getSolution_cap();
            
            cout << "\n\n\n";
            cout << "Finishing post-optimization" << endl;
            solution.print(); 
            
            cout << "Solution Stats" << endl;
            solution.statsDistances();
            solution.printStatsDistances();

            solution.saveAssignment(config.output_filename, "RSSV_" + config.Method_RSSV_fp + "_POSTOPT", elapsed_time_postopt);
            solution.saveResults(config.output_filename, elapsed_time_postopt, 0, "RSSV_" + config.Method_RSSV_fp + "_POSTOPT");

            cout << "Final total elapsed time: " << elapsed_time_postopt << "s\n";

        }
    }




}



Solution_std methods_PMP(const shared_ptr<Instance>& instance, const Config& config, double external_time) {
    Solution_std solution;
    cout << "-------------------------------------------------\n";

    if (config.Method == "EXACT_PMP") {
        cout << "Exact method PMP\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "PMP");
        pmp.setCoverModel(config.cover_mode, instance->getTypeSubarea());
        pmp.setCoverModel_n2(config.cover_mode_n2, instance->getTypeSubarea_n2());
        pmp.run(config.Method);
        pmp.saveVars(config.output_filename, config.Method);
        pmp.saveResults(config.output_filename, config.Method);
        solution = pmp.getSolution_std();
    } else if (config.Method == "TB_PMP") {
        cout << "TB heuristic - standard PMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, config.seed);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setTimeLimit(config.CLOCK_LIMIT);
        solution = heuristic.run(true, UB_MAX_ITER);
    } else if (config.Method == "VNS_PMP") {
        cout << "VNS heuristic - PMP\n";
        cout << "-------------------------------------------------\n";
        VNS heuristic(instance, config.seed);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setExternalTime(external_time); // external time is not used in PMP
        solution = heuristic.runVNS_std(true, UB_MAX_ITER);
    } else {
        cout << "[ERROR] Method not found" << endl;
        exit(1);
    }
    return solution;
}
Solution_cap methods_CPMP(const shared_ptr<Instance>& instance, const Config& config, double external_time) {
    Solution_cap solution;
    Solution_MAP solution_map(instance);
    bool add_InitialSolution_RSSV = false;

    string Method = config.Method;
    if (Method == "RSSV") {
        Method = config.Method + "_" + config.Method_RSSV_fp;
    }


    cout << "-------------------------------------------------\n";
    if (Method == "EXACT_CPMP" || Method == "RSSV_EXACT_CPMP") {
        cout << "Exact method cPMP continuos\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "CPMP");
        pmp.setGenerateReports(true);
        pmp.setCoverModel(config.cover_mode, instance->getTypeSubarea());
        pmp.setCoverModel_n2(config.cover_mode_n2, instance->getTypeSubarea_n2());
        pmp.setTimeLimit(config.CLOCK_LIMIT_CPLEX - external_time);
        if (config.CLOCK_LIMIT_CPLEX == 0) {pmp.setTimeLimit(0);} // no time limit

        if (add_InitialSolution_RSSV && Method == "RSSV_EXACT_CPMP") {
            auto vet_locs = instance->getVotedLocs();
            unordered_set<uint_t> init_p(vet_locs.begin(), vet_locs.begin() + instance->get_p());
            Solution_cap init_sol(instance, init_p, "GAPrelax");
            if (init_sol.getFeasibility()) {
                pmp.setMIPStartSolution(init_sol);
                init_sol.print();
            } else {
                cout << "Initial solution not feasible\n";
            }
        }

        pmp.run(Method);
        pmp.saveVars(config.output_filename, Method);
        pmp.saveResults(config.output_filename, Method);
        solution = pmp.getSolution_cap();
    } else if (Method == "EXACT_CPMP_BIN" || Method == "RSSV_EXACT_CPMP_BIN") {
        cout << "Exact method cPMP binary\n";
        cout << "-------------------------------------------------\n";
        PMP pmp(instance, "CPMP", true);
        pmp.setGenerateReports(true);
        pmp.setCoverModel(config.cover_mode, instance->getTypeSubarea());
        pmp.setCoverModel_n2(config.cover_mode_n2, instance->getTypeSubarea_n2());
        pmp.setTimeLimit(config.CLOCK_LIMIT_CPLEX - external_time);
        if (config.CLOCK_LIMIT_CPLEX == 0) {pmp.setTimeLimit(0);} // no time limit

        if (add_InitialSolution_RSSV && Method == "RSSV_EXACT_CPMP_BIN") {
            auto vet_locs = instance->getVotedLocs();
            unordered_set<uint_t> init_p(vet_locs.begin(), vet_locs.begin() + instance->get_p());
            Solution_cap init_sol(instance, init_p, "GAP");
            if (init_sol.getFeasibility()) {
                pmp.setMIPStartSolution(init_sol);
                init_sol.print();
            } else {
                cout << "Initial solution not feasible\n";
            }
        }

        pmp.run(Method);
        pmp.saveVars(config.output_filename, Method);
        pmp.saveResults(config.output_filename, Method);
        solution = pmp.getSolution_cap();
    } else if (Method == "TB_CPMP" || Method == "RSSV_TB_CPMP") {
        cout << "TB heuristic - cPMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, config.seed);
        heuristic.setSolutionMap(solution_map);
        heuristic.setGenerateReports(true);
        heuristic.setMethod(Method);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setTimeLimit(config.CLOCK_LIMIT);
        solution = heuristic.run_cap(true, UB_MAX_ITER);
    } else if (Method == "VNS_CPMP" || Method == "RSSV_VNS_CPMP") {
        cout << "VNS heuristic - cPMP\n";
        cout << "-------------------------------------------------\n";
        VNS heuristic(instance, config.seed);
        heuristic.setSolutionMap(solution_map);
        heuristic.setGenerateReports(true);
        heuristic.setMethod(Method);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        heuristic.setExternalTime(external_time);

        if (add_InitialSolution_RSSV && Method == "RSSV_VNS_CPMP") {
            auto vet_locs = instance->getVotedLocs();
            unordered_set<uint_t> init_p(vet_locs.begin(), vet_locs.begin() + instance->get_p());
            Solution_cap init_sol(instance, init_p, "GAP");
            if (init_sol.getFeasibility()) {
                heuristic.setInitialSolution(init_sol);
                init_sol.print();
            } else {
                cout << "Initial solution not feasible\n";
            }
        }

        solution = heuristic.runVNS_cap(Method, true, UB_MAX_ITER);

        if (solution.isSolutionFeasible()) {
            cout << "Solution feasible\n";
        } else {
            cout << "Solution not feasible\n";
        }

    } else if (Method == "GAPrelax" || Method == "GAP") {
        cout << Method << " - cPMP\n";
        cout << "-------------------------------------------------\n";
        TB heuristic(instance, config.seed);
        heuristic.setMethod(Method);
        heuristic.setCoverMode(config.cover_mode);
        heuristic.setCoverMode_n2(config.cover_mode_n2);
        auto solution = heuristic.fixedCapSolution(Method);
        cout << "Final solution:\n";
        solution.print();
        exit(1);

    } else {
        cout << "[ERROR] Method not found" << endl;
        exit(1);
    }

    if (Method != "EXACT_CPMP" && Method != "EXACT_CPMP_BIN" && Method != "RSSV_EXACT_CPMP" && Method != "RSSV_EXACT_CPMP_BIN" && Method != "GAPrelax" && Method != "GAP") {
        solution.setCoverMode(config.cover_mode);
        auto p_loc = solution.get_pLocations();
        auto sol_best = Solution_cap(instance, p_loc, "GAPrelax", config.cover_mode);
        return sol_best;
    }

    auto sol_best = solution;
    sol_best.setCoverMode(config.cover_mode);
    sol_best.setCoverMode_n2(config.cover_mode_n2);

    return sol_best;
}