#include "RSSV.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <random>
#include <chrono>

void printDDE(void){
    cout << "RSSV finished." << endl;
}

RSSV::RSSV(const shared_ptr<Instance>& instance, uint_t seed, uint_t n, uint_t n_cand):instance(instance), n(n), n_cand(n_cand) {
    seed_rssv = seed;
    engine.seed(seed_rssv);

    N = instance->getLocations().size();
    if (n_cand == 0) {
        this->n_cand = n;
    }else{
        this->n_cand = min(n_cand, N);
    }

    M = max(static_cast<uint_t>(1), (LOC_FREQUENCY*N/min(n,N)));


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
    
    M = min(M,thread_cnt);

    num_facilities_subproblem = min(n, N);
    n = num_facilities_subproblem;
    num_customers_subproblem = instance->getCustomers().size();
    // num_customers_subproblem = num_facilities_subproblem;
    p_subproblem = instance->get_p();
    
    cout << "Prob size (N,C): (" << N << "," << instance->getCustomers().size() <<  ")" << endl;
    cout << "sub-Prob size (n,c): (" << num_facilities_subproblem << "," << num_customers_subproblem <<  ")" << endl;
    cout << "sub-Prob p: " << p_subproblem << endl;
    cout << "Subproblems cnt (M): " << M << endl;
    cout << "Final Prob RSSV size (N',C): (" << n_cand << "," << instance->getCustomers().size() <<  ")" << endl;
    this->method_RSSV_sp = method_sp;
    cout << "Seed: " << seed_rssv  << endl;
    if(TIME_LIMIT_SUBPROBLEMS > 0) cout << "Time limit for subproblems: " << TIME_LIMIT_SUBPROBLEMS << endl;
    if(MAX_ITE_SUBPROBLEMS > 0) cout << "Max iterations for subproblems: " << MAX_ITE_SUBPROBLEMS << endl;
    cout << endl;


    if (p_subproblem > num_facilities_subproblem) {
        cout << "[ERROR] The number of facilities is smaller than the number of locations to be selected" << endl;
        cout << "[WARN] Setting n = min(2 * p, N)" << endl;
        n = min(static_cast<uint_t>(2 * p_subproblem), N);
    }

    sem.setCount(thread_cnt);
    cout << "thread cnt: " << thread_cnt << endl << endl;

    double TIME_LIMIT_ALL_SUBPROBLEMS = 2000; // time limit to solve all subproblems

    auto start_time = tick();
    auto start_cpu = chrono::high_resolution_clock::now();
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

        auto elapsed_time = chrono::duration_cast<seconds>(high_resolution_clock::now() - start_cpu).count();
        if (elapsed_time > TIME_LIMIT_ALL_SUBPROBLEMS) {
            cout << "\n[TIMELIMIT] Time limit exceeded to solve all Sub-cPMPs " << endl;
            break;
        }

    }

    cout << "[INFO] All subproblems solved." << endl << endl;
    tock(start_time);

    subSols_avg_dist = subSols_avg_dist / M;
    subSols_std_dev_dist = subSols_std_dev_dist / M;
    cout << "\nStats: \n";
    cout << "Max dist: " << subSols_max_dist << endl;
    cout << "Minmax dist: " << subSols_minmax_dist << endl;
    cout << "Min dist: " << subSols_min_dist << endl;
    cout << "avg of Avg dists: " << subSols_avg_dist << endl;
    cout << "avg Std dev dist: " << subSols_std_dev_dist << endl;
    cout << "Max number of assignments: " << subSols_max_num_assignments << endl;
    cout << endl;

    // auto filtered_cnt = n;
    auto filtered_cnt = min(n_cand,N);
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

    // final_locations = randomLocations(filtered_cnt);
    // cout << endl << endl;
    // cout << "Random locations (test voted -> random) " << filtered_cnt << " locations: ";
    // for (auto fl : final_locations) cout << fl << " ";
    // cout << endl << endl;



    // this->final_voted_locs = final_locations;
    this->final_voted_locs = filterLocations_nonzero(min(n,n_cand));
    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations, instance->getTypeService()));
    filtered_instance->setVotedLocs(filtered_locations);

    cout << " instance threshold dist: " << instance->get_ThresholdDist() << endl;

    subSols_max_dist = min(subSols_max_dist, max_dist_feas);

    if(instance->get_ThresholdDist() > 0)
        filtered_instance->set_ThresholdDist(instance->get_ThresholdDist());
    if (add_threshold_dist){
        if (instance->get_ThresholdDist() > 0)  subSols_max_dist = min(subSols_max_dist, instance->get_ThresholdDist());
        filtered_instance->set_ThresholdDist(subSols_max_dist);
        if(method_sp == "TB_CPMP" || method_sp == "EXACT_CPMP") filtered_instance->set_ThresholdDist(subSols_minmax_dist);
    }

    atexit(printDDE);

    return filtered_instance;
}


Instance returnSampleInstance(string typeSample, uint n, uint m, uint p, uint seed, int thread_id, int num_clusters, shared_ptr<Instance> instance){ 
    if (typeSample == "RANDOM") {
        return instance->sampleSubproblem(n, m, p, seed);
    } else if (typeSample == "KMEANS_CLUSTERS") {
        return instance->sampleSubproblemFromClusters(n, m, p, num_clusters, seed);
    } else {
        cout << "[WARN] Type of sample not found" << endl;
        cout << "Using random sample" << endl;
        return instance->sampleSubproblem(n, m, p, seed);
    }
}

template <typename SolutionType>
void RSSV::solveSubproblemTemplate(int seed, bool isCapacitated) {
    // Use the seed for random number generation
    int thread_id = seed - seed_rssv;
    std::mt19937 gen(seed);
    sem.wait(thread_id);

    cout << "Solving sub-PMP " << thread_id << "/" << M << "..." << endl;
    auto start = tick();
    // Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(), seed);
    // Instance subInstance = instance->sampleSubproblem(num_facilities_subproblem, num_customers_subproblem, p_subproblem, seed);

    string typeSample;
    int num_clusters = 0;

    typeSample = "RANDOM";
    // typeSample = "KMEANS_CLUSTERS";
    num_clusters = int(p_subproblem);
    cout << "Sampling sub-PMP " <<  thread_id << " with " << typeSample << " method" << endl;
    Instance subInstance = returnSampleInstance(typeSample, num_facilities_subproblem, num_customers_subproblem, p_subproblem, seed, thread_id, num_clusters, instance);


    // int num_cluster = thread_id;
    // Instance subInstance = instance->getSubproblemFromClusters(num_cluster);


    subInstance.set_isWeightedObjFunc(false);

    double time_limit_subproblem = 0; // no time limit = 0
    uint_t MAX_ITER_SUBP = UB_MAX_ITER; // Upper Bound for the number of iterations in the subproblem; 
    
    if (MAX_ITE_SUBPROBLEMS > 0) MAX_ITER_SUBP = MAX_ITE_SUBPROBLEMS;
    if (TIME_LIMIT_SUBPROBLEMS > 0) time_limit_subproblem = TIME_LIMIT_SUBPROBLEMS;
    

    bool verb = false;

    SolutionType sol;
    if (checkClock()) {
        if (method_RSSV_sp == "EXACT_PMP" || (isCapacitated && method_RSSV_sp == "EXACT_CPMP") || (isCapacitated && method_RSSV_sp == "EXACT_CPMP_BIN")) {
            PMP pmp(make_shared<Instance>(subInstance), isCapacitated ? "CPMP" : "PMP", method_RSSV_sp == "EXACT_CPMP_BIN");
            pmp.setCoverModel(cover_mode, instance->getTypeSubarea());
            pmp.setCoverModel_n2(cover_mode_n2, instance->getTypeSubarea_n2());
            if (time_limit_subproblem > 0) pmp.setTimeLimit(time_limit_subproblem);
            pmp.setAddBreakCallback(true);
            // pmp.setGenerateReports(true);
            pmp.setDisplayCPLEX(false);
            pmp.run(method_RSSV_sp);
            if constexpr (std::is_same_v<SolutionType, Solution_std>) {
                sol = pmp.getSolution_std();
            } else if constexpr (std::is_same_v<SolutionType, Solution_cap>) {
                sol = pmp.getSolution_cap();
            }
        } else if (method_RSSV_sp == "TB_PMP" || (isCapacitated && method_RSSV_sp == "TB_CPMP")) {
            TB heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            heuristic.setCoverMode_n2(cover_mode_n2);
            // if (num_clusters>0) heuristic.setRandomClusterInitialSolution(true);
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
        cout << "Solution " << seed << ": ";
        sol.print();
        processSubsolutionScores(make_shared<SolutionType>(sol));
        processSubsolutionDists(make_shared<SolutionType>(sol));
        if (method_RSSV_sp == "TB_PMP") processSubsolutionCapacities(make_shared<SolutionType>(sol));
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
    uint_t max_num_assignments_local = solution->getMaxNumberAssignments();
    

    dist_mutex.lock();
    subSols_max_dist = max(subSols_max_dist, max_dist_local);
    subSols_minmax_dist = min(subSols_minmax_dist, max_dist_local); // need to change the subSols_max_dist initial
    subSols_min_dist = min(subSols_min_dist, min_dist_local);
    subSols_avg_dist += avg_dist_local;
    subSols_std_dev_dist += std_dev_dist_local;
    subSols_max_num_assignments = max(subSols_max_num_assignments, max_num_assignments_local);
    dist_mutex.unlock();
}


template <typename SolutionType>
void RSSV::processSubsolutionCapacities(shared_ptr<SolutionType> solution) {
    // check for each assigment if capacity is bigger than the sum of the demands
    cap_check_mutex.lock();
    if(solution->capacitiesAssigmentRespected(this->instance)){
        // capacities_respected = true;
        cout << "Solution Capacities respected" << endl;
        solution->statsDistances();
            cout << "\nStats: \n";
            cout << "Max dist: " << solution->getMaxDist() << endl;
            cout << "Min dist: " << solution->getMinDist() << endl;
            cout << "avg of Avg dists: " << solution->getAvgDist() << endl;
            cout << "avg Std dev dist: " << solution->getStdDevDist() << endl;
            cout << "Max number of assignments: " << solution->getMaxNumberAssignments() << endl;
        if (max_dist_feas < solution->getMaxDist()) max_dist_feas = solution->getMaxDist();
    }else{
        // capacities_respected = false;
        cout << "Solution Capacities NOT respected" << endl;
    }
    cap_check_mutex.unlock();
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

vector<uint_t> RSSV::filterLocations_nonzero(uint_t cnt) {
    vector<pair<uint_t, double>> weights_vec;
    for (auto w:weights) { // put pairs loc, weight in a vector
        weights_vec.emplace_back(w);
    }
    sort(weights_vec.begin(), weights_vec.end(), cmp); // sort by weight
    reverse(weights_vec.begin(), weights_vec.end()); // reverse (high to low weight now)

    // cont non zero weights
    uint_t cnt_nonzero = 0;
    for (auto w:weights_vec) {
        if (w.second > 0) {
            cnt_nonzero++;
        }
    }
    cout << "First " << min(cnt,cnt_nonzero) << " voting non zero weights (sorted): ";
    vector<uint_t> filtered_locs; // Extract at most cnt first locations
    uint_t cnt_ = 0;
    for (auto w:weights_vec) {
        if (w.second > 0) {
            printf("%d(%.2f) ", w.first, w.second);
            filtered_locs.emplace_back(w.first);
            cnt_++;
            if (cnt_ == cnt) {
                break;
            }
        }
    }
    cout << endl;

    return filtered_locs;
}

vector<uint_t> RSSV::randomLocations(uint_t cnt) {
    vector<uint_t> locations;
    vector<uint_t> locs = instance->getLocations();
    shuffle(locs.begin(), locs.end(), engine);
    for (uint_t i = 0; i < cnt; ++i) {
        locations.emplace_back(locs[i]);
    }
    return locations;
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

vector<uint_t> RSSV::getFinalVotedLocs(){
    return this->final_voted_locs;
}