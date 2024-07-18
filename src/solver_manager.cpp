#include "solver_manager.hpp"

SolverManager::SolverManager(const Config& config, const Instance& instance)
    : config(config), instance(instance) {}

void SolverManager::solveProblem() {

    std::cout << "-------------------------------------------------\n";

    if (config.Method == "EXACT_PMP" || config.Method == "TB_PMP" || config.Method == "VNS_PMP") {
        auto start_time = high_resolution_clock::now();
        Solution_std solution = methods_PMP(std::make_shared<Instance>(instance), config, 0);
        auto elapsed_time = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();

        printSolution(solution, elapsed_time);
        saveSolution(solution, elapsed_time);

        if (config.post_opt_method != "null") {
            runPostOptimization(solution, elapsed_time, start_time);
        }
    } else if (config.Method == "EXACT_CPMP" || config.Method == "EXACT_CPMP_BIN" ||
               config.Method == "TB_CPMP" || config.Method == "VNS_CPMP" ||
               config.Method == "GAPrelax" || config.Method == "GAP") {

        solveCPMP();
    } else if (config.Method == "RSSV") {
        solveRSSV();
    }
}

void SolverManager::solveCPMP() {
    using namespace std::chrono;

    auto start_time = high_resolution_clock::now();
    Solution_cap solution = methods_CPMP(std::make_shared<Instance>(instance), config, 0);
    auto elapsed_time_total = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();

    printSolution(solution, elapsed_time_total);
    saveSolution(solution, elapsed_time_total);

    if (config.post_opt_method != "null") {
        runPostOptimization(solution, elapsed_time_total, start_time);
    }
}

void SolverManager::solveRSSV() {
    using namespace std::chrono;

    std::cout << "RSSV heuristic\n-------------------------------------------------\n";
    RSSV metaheuristic(std::make_shared<Instance>(instance), config.seed, SUB_PMP_SIZE);
    metaheuristic.setCoverMode(config.cover_mode);
    metaheuristic.setCoverMode_n2(config.cover_mode_n2);
    metaheuristic.setMAX_ITE_SUBPROBLEMS(config.MAX_ITE_SUBPROB_RSSV);
    metaheuristic.setTIME_LIMIT_SUBPROBLEMS(config.CLOCK_LIMIT_SUBPROB_RSSV);
    metaheuristic.setAddThresholdDist(config.add_threshold_distance_rssv);
    CLOCK_THREADED = true;
    auto start_time_total = high_resolution_clock::now();

    std::shared_ptr<Instance> filtered_instance;
    if (config.Method_RSSV_sp == "EXACT_PMP" || config.Method_RSSV_sp == "TB_PMP" || config.Method_RSSV_sp == "VNS_PMP") {
        filtered_instance = metaheuristic.run(THREAD_NUMBER, config.Method_RSSV_sp);
    } else if (config.Method_RSSV_sp == "EXACT_CPMP" || config.Method_RSSV_sp == "EXACT_CPMP_BIN" ||
               config.Method_RSSV_sp == "TB_CPMP" || config.Method_RSSV_sp == "VNS_CPMP") {
        filtered_instance = metaheuristic.run_CAP(THREAD_NUMBER, config.Method_RSSV_sp);
    } else {
        std::cerr << "[ERROR] Method subproblem RSSV not found" << std::endl;
        exit(1);
    }

    filtered_instance->setCoverModel(config.cover_mode);
    filtered_instance->setCoverModel_n2(config.cover_mode_n2);
    filtered_instance->set_isWeightedObjFunc(config.IsWeighted_ObjFunc);

    std::cout << "Final instance parameters:\n";
    filtered_instance->print();

    std::cout << "-------------------------------------------------\n";
    std::cout << "Final Problem RSSV heuristic\n-------------------------------------------------\n";

    if (config.Method_RSSV_fp == "EXACT_PMP" || config.Method_RSSV_fp == "TB_PMP" || config.Method_RSSV_fp == "VNS_PMP") {
        solveFinalProblem<methods_PMP>(filtered_instance, start_time_total);
    } else if (config.Method_RSSV_fp == "EXACT_CPMP" || config.Method_RSSV_fp == "EXACT_CPMP_BIN" ||
               config.Method_RSSV_fp == "TB_CPMP" || config.Method_RSSV_fp == "VNS_CPMP") {
        solveFinalProblem<methods_CPMP>(filtered_instance, start_time_total);
    }
}

template <typename SolverFunc>
void SolverManager::solveFinalProblem(const std::shared_ptr<Instance>& filtered_instance, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time_total) {
    using namespace std::chrono;

    auto start_time = high_resolution_clock::now();
    auto solution = SolverFunc(filtered_instance, config, duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());
    auto elapsed_time = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();

    printSolution(solution, elapsed_time);
    saveSolution(solution, elapsed_time);

    std::cout << "\n\nSolution Stats\n";
    solution.statsDistances();
    solution.printStatsDistances();

    if (config.post_opt_method != "null") {
        runPostOptimization(solution, elapsed_time, start_time_total);
    }
}

template <typename SolutionType>
void SolverManager::runPostOptimization(SolutionType& solution, double elapsed_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time_total) {
    using namespace std::chrono;

    std::cout << "\n\n-------------------------------------------------\n";
    std::cout << "Post Optimization\n-------------------------------------------------\n";
    auto time_left = config.CLOCK_LIMIT - elapsed_time;
    std::cout << "Time for post-optimization: " << time_left << "s\n";
    PostOptimization postOptimization(config.post_opt_method, instance, solution);
    postOptimization.set_time_limit(time_left);
    postOptimization.run();
    auto elapsed_time_postopt = duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count();
    solution = postOptimization.getSolution();
    solution.saveAssignment(config.output_filename, config.Method + "_POSTOPT", elapsed_time_postopt);
    solution.saveResults(config.output_filename, elapsed_time_postopt, 0, config.Method + "_POSTOPT");
}

template <typename SolutionType>
void SolverManager::printSolution(const SolutionType& solution, double elapsed_time) const {
    std::cout << "\nFinal solution:\n";
    solution.print();
    solution.statsDistances();
    std::cout << "Final total elapsed time: " << elapsed_time << "s\n";
}

template <typename SolutionType>
void SolverManager::saveSolution(const SolutionType& solution, double elapsed_time) const {
    solution.saveAssignment(config.output_filename, config.Method, elapsed_time);
    solution.saveResults(config.output_filename, elapsed_time, 0, config.Method);
}