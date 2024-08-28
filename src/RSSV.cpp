#include "RSSV.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <random>

void printDDE(void){
    cout << "RSSV finished." << endl;
}

RSSV::RSSV(const shared_ptr<Instance>& instance, uint_t seed, uint_t n):instance(instance), n(n) {
    seed_rssv = seed;
    engine.seed(seed_rssv);

    N = instance->getLocations().size();
    M = max(static_cast<uint_t>(1), (LOC_FREQUENCY*N/n));

    for (auto loc : instance->getLocations()) {
        weights[loc] = DEFAULT_WEIGHT;
    }
}

shared_ptr<Instance> RSSV::run(uint_t thread_cnt, const string& method_sp) {
    return run_impl(thread_cnt, method_sp, false);
}
shared_ptr<Instance> RSSV::run_CAP(uint_t thread_cnt, const string& method_sp) {
    return run_impl(thread_cnt, method_sp, true);
}
shared_ptr<Instance> RSSV::run_impl(uint_t thread_cnt, const string& method_sp, bool is_cap) {
    cout << "RSSV running...\n";
    cout << "cPMP size (N): " << N << endl;
    cout << "sub-cPMP size (n): " << min(n, N) << endl;
    cout << "Subproblems cnt (M): " << M << endl;
    cout << "p: " << instance->get_p() << endl;
    this->method_RSSV_sp = method_sp;
    // cout << "Method to solve the Subproblems: " << method_RSSV_sp << endl;
    cout << "Seed: " << seed_rssv  << endl;
    if(TIME_LIMIT_SUBPROBLEMS > 0) cout << "Time limit for subproblems: " << TIME_LIMIT_SUBPROBLEMS << endl;
    if(MAX_ITE_SUBPROBLEMS > 0) cout << "Max iterations for subproblems: " << MAX_ITE_SUBPROBLEMS << endl;
    
    cout << endl;


    if (instance->get_p() > min(n, N)) {
        cout << "[ERROR] The number of facilities is smaller than the number of locations to be selected" << endl;
        cout << "[WARN] Setting n = min(1.5 * p, N)" << endl;
        n = min(static_cast<uint_t>(1.5 * instance->get_p()), N);
    }

    sem.setCount(thread_cnt);
    cout << "thread cnt: " << thread_cnt << endl << endl;

    auto start_time = tick();
    vector<thread> threads;
    for (uint_t i = 1; i <= M; i += thread_cnt) {
        for (uint_t j = 0; j < thread_cnt && (i + j) <= M; ++j) {
            int seed_thread = seed_rssv + i + j;
            cout << "Thread " << i + j << " created with seed " << seed_thread << endl;
            
            if (is_cap) {
                threads.emplace_back([this, seed_thread]() {
                    std::mt19937 gen(seed_thread); // Local to each thread
                    this->solveSubproblemTemplate<Solution_cap>(seed_thread, true);
                });
            } else {
                threads.emplace_back([this, seed_thread]() {
                    std::mt19937 gen(seed_thread); // Local to each thread
                    this->solveSubproblemTemplate<Solution_std>(seed_thread, false);
                });
            }

        }

        for (auto &th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }
        threads.clear();
    }

    cout << "[INFO] All subproblems solved." << endl << endl;
    tock(start_time);

    subSols_avg_dist = subSols_avg_dist / M;
    subSols_std_dev_dist = subSols_std_dev_dist / M;
    cout << "\nStats: \n";
    cout << "Max dist: " << subSols_max_dist << endl;
    cout << "Min dist: " << subSols_min_dist << endl;
    cout << "avg of Avg dists: " << subSols_avg_dist << endl;
    cout << "avg Std dev dist: " << subSols_std_dev_dist << endl;
    cout << endl;


    auto filtered_cnt = n;
    auto filtered_locations = filterLocations(filtered_cnt);
    cout << endl << endl;
    cout << "Filtered " << filtered_cnt << " locations: ";
    for (auto fl : filtered_locations) cout << fl << " ";
    cout << endl << endl;

    auto prioritized_locations = extractPrioritizedLocations(LOC_PRIORITY_CNT);
    cout << "Extracted " << prioritized_locations.size() << " prioritized locations: ";
    for (auto pl : prioritized_locations) cout << pl << " ";
    cout << endl << endl;

    for (auto fl : filtered_locations) prioritized_locations.insert(fl);
    vector<uint_t> final_locations(prioritized_locations.begin(), prioritized_locations.end());

    bool extract_fixed_locations = false;
    if (extract_fixed_locations) {
        cout << "Replace with fixed locations: ";
        final_locations = extractFixedLocations(final_locations);
        cout << endl << endl;
        cout << "Final " << filtered_cnt << " locations: ";
        for (auto fl : final_locations) cout << fl << " ";
        cout << endl << endl;
        cout << "Size of final locations: " << final_locations.size() << endl << endl;
    }

    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations, instance->getTypeService()));
    filtered_instance->setVotedLocs(filtered_locations);

    
    if (add_threshold_dist) {
        filtered_instance->set_ThresholdDist(subSols_max_dist+subSols_std_dev_dist);
    }

    atexit(printDDE);

    return filtered_instance;
}


template <typename SolutionType>
void RSSV::solveSubproblemTemplate(int seed, bool isCapacitated) {
    // Use the seed for random number generation
    int thread_id = seed - seed_rssv;
    // std::mt19937 gen(seed);
    sem.wait(thread_id);

    cout << "Solving sub-PMP " << thread_id << "/" << M << "..." << endl;
    auto start = tick();
    // uint_t nu = this->n;
    uint_t num_customers = instance->getCustomers().size();
    uint_t p_subproblem = min(instance->get_p(),uint_t(100));
    // Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(), seed);
    Instance subInstance = instance->sampleSubproblem(n, n, p_subproblem, seed);
    subInstance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
    
    if (instance->get_p() > n) {
        cout << "[ERROR] The number of facilities is greater than the number of locations to be selected" << endl;
        exit(1);
    }

    double time_limit_subproblem = 0; // no time limit = 0
    uint_t MAX_ITER_SUBP = UB_MAX_ITER; // Upper Bound for the number of iterations in the subproblem; 
    
    if (MAX_ITE_SUBPROBLEMS > 0) MAX_ITER_SUBP = MAX_ITE_SUBPROBLEMS;
    if (TIME_LIMIT_SUBPROBLEMS > 0) time_limit_subproblem = TIME_LIMIT_SUBPROBLEMS;
    
    
    bool verb = false;

    SolutionType sol;
    if (checkClock()) {
        if (method_RSSV_sp == "EXACT_PMP" || (isCapacitated && method_RSSV_sp == "EXACT_CPMP")) {
            PMP pmp(make_shared<Instance>(subInstance), isCapacitated ? "CPMP" : "PMP");
            pmp.setCoverModel(cover_mode, instance->getTypeSubarea());
            pmp.setCoverModel_n2(cover_mode_n2, instance->getTypeSubarea_n2());
            if (time_limit_subproblem > 0) pmp.setTimeLimit(time_limit_subproblem);
            if constexpr (std::is_same_v<SolutionType, Solution_std>) {
                sol = pmp.getSolution_std();
            } else if constexpr (std::is_same_v<SolutionType, Solution_cap>) {
                sol = pmp.getSolution_cap();
            }
        } else if (method_RSSV_sp == "TB_PMP" || (isCapacitated && method_RSSV_sp == "TB_CPMP")) {
            TB heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            heuristic.setCoverMode_n2(cover_mode_n2);
            if (time_limit_subproblem > 0) heuristic.setTimeLimit(time_limit_subproblem);
            if constexpr (std::is_same_v<SolutionType, Solution_std>) {
                sol = heuristic.run(verb, MAX_ITER_SUBP);
            } else if constexpr (std::is_same_v<SolutionType, Solution_cap>) {
                sol = heuristic.run_cap(verb, MAX_ITER_SUBP);
            }
        } else if (method_RSSV_sp == "VNS_PMP" || (isCapacitated && method_RSSV_sp == "VNS_CPMP")) {
            VNS heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            heuristic.setCoverMode_n2(cover_mode_n2);
            // if (time_limit_subproblem > 0) heuristic.setTimeLimit(time_limit_subproblem); // not implemented yet in VNS
            if constexpr (std::is_same_v<SolutionType, Solution_std>) {
                sol = heuristic.runVNS_std(verb, MAX_ITER_SUBP);
            } else if constexpr (std::is_same_v<SolutionType, Solution_cap>) {
                sol = heuristic.runVNS_cap(method_RSSV_sp, verb, MAX_ITER_SUBP);
            }
        } else {
            cout << "Method to solve the Subproblems: " << method_RSSV_sp << " not found" << endl;
            exit(1);
        }

        if (VERBOSE) cout << "Solution " << seed << ": ";
        if (VERBOSE) sol.print();
        processSubsolutionScores(make_shared<SolutionType>(sol));
        processSubsolutionDists(make_shared<SolutionType>(sol));
        if (VERBOSE) tock(start);
        sem.notify(seed);
    } else {
        cout << "[TIMELIMIT]  Time limit exceeded to solve Sub-cPMPs " << endl;
    }

}

/*
 * Extract voting weights from a subproblem solution.
 * Distance to closest locations is determined from closest customer, NOT from the location in the solution (as it should be).
 */
template <typename SolutionType>
void RSSV::processSubsolutionScores(shared_ptr<SolutionType> solution) {
    for (auto loc_sol : solution->get_pLocations()) {
        // get closest customer in orig. instance
        auto cust_cl = instance->getClosestCust(loc_sol);
        // evaluate voting score increment for all locations in orig. instance
        weights_mutex.lock();
        for (auto loc : instance->getLocations()) {
            if (loc == loc_sol) {
                weights[loc] += 1;
            } else {
                // weights[loc] += instance->getVotingScore(loc, cust_cl);
                weights[loc] += instance->getVotingScore(loc, loc_sol);
            }
        }
        weights_mutex.unlock();
    }
}

template <typename SolutionType>
void RSSV::processSubsolutionDists(shared_ptr<SolutionType> solution) {
    solution->statsDistances();
    dist_t max_dist_local = solution->getMaxDist();
    dist_t min_dist_local = solution->getMinDist();
    dist_t avg_dist_local = solution->getAvgDist();
    dist_t std_dev_dist_local = solution->getStdDevDist();
    

    dist_mutex.lock();
    subSols_max_dist = max(subSols_max_dist, max_dist_local);
    subSols_min_dist = min(subSols_min_dist, min_dist_local);
    subSols_avg_dist += avg_dist_local;
    subSols_std_dev_dist += std_dev_dist_local;
    dist_mutex.unlock();
}


bool cmp(pair<uint_t, double>& a,
         pair<uint_t, double>& b){
    return a.second < b.second;
}
/*
 * Filter locations for the final filtered instance.
 * First cnt(=n) locations with the highest weight are extracted.
 */
vector<uint_t> RSSV::filterLocations(uint_t cnt) {
    vector<pair<uint_t, double>> weights_vec;
    for (auto w:weights) { // put pairs loc, weight in a vector
        weights_vec.emplace_back(w);
    }
    sort(weights_vec.begin(), weights_vec.end(), cmp); // sort by weight
    reverse(weights_vec.begin(), weights_vec.end()); // reverse (high to low weight now)

    cout << "First " << cnt << " voting weights (sorted): ";
    vector<uint_t> filtered_locs; // Extract at most cnt first locations
    uint_t cnt_ = 0;
    for (auto w:weights_vec) {
        printf("%d(%.2f) ", w.first, w.second);
        filtered_locs.emplace_back(w.first);
        cnt_++;
        if (cnt_ == cnt) {
            break;
        }
    }
    cout << endl;

    return filtered_locs;
}

unordered_set<uint_t> RSSV::extractPrioritizedLocations(uint_t min_cnt) {
    unordered_set<uint_t> locations;
    for (auto c:instance->getCustomers()) {
        uint_t cnt = 0;
        for (auto l:instance->getLocations()) {
            if (DEFAULT_DISTANCE - instance->getRealDist(l, c) > TOLERANCE_OBJ) cnt++;
        }
        if (cnt <= min_cnt) {
            for (auto l:instance->getLocations()) {
                if (DEFAULT_DISTANCE - instance->getRealDist(l, c) > TOLERANCE_OBJ) locations.insert(l);
            }
        }
    }
    return locations;
}

unordered_set<uint_t> get_p_values_solution(const std::string& file_path) {
        std::unordered_set<uint_t> unique_values; // Set to store unique values
        std::unordered_set<int> seen_values; // Set to track seen values
        std::ifstream file(file_path);

        if (!file.is_open()) {
            std::cerr << "Error opening file: " << file_path << std::endl;
            return unique_values;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string value;

            while (iss >> value) {
                try {
                    uint_t numeric_value = std::stoul(value); // Convert to unsigned int

                    if (seen_values.find(numeric_value) != seen_values.end()) {
                        return unique_values; // Return if numeric_value is duplicated
                    } else {
                        unique_values.insert(numeric_value); // Add to result set
                        seen_values.insert(numeric_value); // Mark as seen
                    }
                } catch (const std::invalid_argument& e) {
                    // Handle non-numeric values if needed
                    continue;
                }
            }
        }

        return unique_values;
    }
vector<uint_t> RSSV::extractFixedLocations(vector<uint_t> vet_locs) {

    auto final_locs = vet_locs;

    string name_file="";
    if (instance->getTypeService() == "pr2392_020")
        name_file = "solucao.pr2392_020_2235376.txt";
    if (instance->getTypeService() == "pr2392_075")
        name_file = "solucao.pr2392_075_1092294.txt";
    if (instance->getTypeService() == "pr2392_150")
        name_file = "solucao.pr2392_150_711111.txt";
    if (instance->getTypeService() == "pr2392_300")
        name_file = "solucao.pr2392_300_458145.txt";
    if (instance->getTypeService() == "pr2392_500")
        name_file = "solucao.pr2392_500_316042.txt";
    string path_sols_lit = "./data/Literature/solutions_lit/" + name_file;
    // cout << "path_sols_lit: " << path_sols_lit << "\n";
    auto p_locations = get_p_values_solution(path_sols_lit);


    // add p locations if doesn't exist in final_locs
    vector<uint_t> not_in_final_locs;
    vector<uint_t> in_final_locs;
    int cont = 0;
    for (auto p_loc:p_locations) {
        if (find(final_locs.begin(), final_locs.end(), p_loc) == final_locs.end()) {
            not_in_final_locs.push_back(p_loc);
            cont++;
        } else{
            in_final_locs.push_back(p_loc);
        }

    }

    // ellimante all ellement in p_locations that are in final_locs
    for (auto loc:in_final_locs) {
        final_locs.erase(remove(final_locs.begin(), final_locs.end(), loc), final_locs.end());
    }

    // ellimine cont ellements in final_locs that are not in p_locations
    for (int i=0;i<cont;i++){ 
        final_locs.pop_back();
    }
            
    // add all ellements in p locations that are not in final_locs
    for (auto loc:p_locations) {
        final_locs.push_back(loc);
    }




    cout << "\n p solution: ";
    for (auto loc:p_locations) {
        cout << loc << " ";
    }
    cout << endl;
    cout << "\nellements already in voting locations: ";
    for (auto loc:in_final_locs) {
        cout << loc << " ";
    }
    cout << endl;
    cout << "\nellements not in voting locations: ";
    for (auto loc:not_in_final_locs) {
        cout << loc << " ";
    }
    cout << endl;

    cout << "Replaced " << cont << " fixed locations" << endl;

    return final_locs;
    
}

void RSSV::setTIME_LIMIT_SUBPROBLEMS(dist_t time_limit) {
    TIME_LIMIT_SUBPROBLEMS = time_limit;
}
void RSSV::setMAX_ITE_SUBPROBLEMS(uint_t max_ite) {
    MAX_ITE_SUBPROBLEMS = max_ite;
}