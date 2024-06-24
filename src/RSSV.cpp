#include "RSSV.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <random>

void printDDE(void){
    cout << "RSSV finished." << endl;
}

RSSV::RSSV(const shared_ptr<Instance>& instance, uint_t seed, uint_t n):instance(instance), n(n) {
    engine.seed(seed);
    seed_rssv = seed;
    N = instance->getLocations().size();
    M = max(static_cast<uint_t>(1), (LOC_FREQUENCY*N/n));

    for (auto loc : instance->getLocations()) {
        weights[loc] = DEFAULT_WEIGHT;
    }
}

/*
 * RSSV metaheuristic implementation.
 */
shared_ptr<Instance> RSSV::run(uint_t thread_cnt, const string& method_sp) {
    cout << "RSSV running...\n";
    cout << "cPMP size (N): " << N << endl;
    cout << "sub-cPMP size (n): " << min(n,N) << endl;
    cout << "Subproblems cnt (M): " << M << endl << endl;
    this->method_RSSV_sp = method_sp;
    cout << "Method to solve the Subproblems: " << method_RSSV_sp  << endl;

    if (instance->get_p() > min(n,N)){
        cout << "[ERROR] The number of facilities is smaller than the number of locations to be selected" << endl;
        cout << "[WARN]  Setting n = min(1.5 * p, N)" << endl;
        n = min(static_cast<uint_t>(1.5*instance->get_p()), N);
        // exit(1);
    }

    sem.setCount(thread_cnt); // limit max no. of threads run in parallel
    cout << "thread cnt:  " << thread_cnt << endl;
    cout << "\n\n";

    auto start_time = tick();
    vector<thread> threads; // spawn M threads
    for (uint_t i = 1; i <= M; i += thread_cnt) {
        for (uint_t j = 0; j < thread_cnt && (i + j) <= M; ++j) {
            // cout << "Thread " << i + j << " created with seed " << i+j + seed_rssv << endl;
            threads.emplace_back(&RSSV::solveSubproblem, this, i+j + seed_rssv);
        }
        // Wait for the current batch of threads to finish before starting the next batch
        for (auto &th : threads) {
            th.join();
        }
        // Clear the threads vector for the next batch
        threads.clear();
    }


    cout << "[INFO] All subproblems solved."  << endl << endl;
    tock(start_time);

    // auto filtered_cnt = max(n, FILETRING_SIZE * instance->get_p());
    auto filtered_cnt = n;
    auto filtered_locations = filterLocations(filtered_cnt); // Filter n locations according to voting weights
    cout << "\n\nFiltered " << filtered_cnt << " locations: ";
    for (auto fl:filtered_locations) cout << fl << " ";
    cout << endl << endl;

    auto prioritized_locations = extractPrioritizedLocations(LOC_PRIORITY_CNT);
    cout << "\n\nExtracted " << prioritized_locations.size() << " prioritized locations: ";
    for (auto pl:prioritized_locations) cout << pl << " ";
    cout << endl << endl;

    for (auto fl:filtered_locations) prioritized_locations.insert(fl);
    vector<uint_t> final_locations (prioritized_locations.begin(), prioritized_locations.end());



    bool extract_fixed_locations = false;
    if (extract_fixed_locations){
        cout << "Replace with fixed locations: ";
        final_locations = extractFixedLocations(final_locations);
        cout << endl << endl;
        cout << "\n\nFinal " << filtered_cnt << " locations: ";
        for (auto fl:final_locations) cout << fl << " ";
        cout << endl << endl;
        // size of final locations
        cout << "Size of final locations: " << final_locations.size() << endl << endl;
    }


    //stats
    subSols_avg_dist = subSols_avg_dist/M;
    // subSols_std_dev_dist = sqrt(subSols_std_dev_dist/M - subSols_avg_dist*subSols_avg_dist);
    subSols_std_dev_dist = subSols_std_dev_dist/M;
    cout << "\nStats: \n";
    cout << "Max dist: " << subSols_max_dist << endl;
    cout << "Min dist: " << subSols_min_dist << endl;
    cout << "avg of Avg dists: " << subSols_avg_dist << endl;
    cout << "avg Std dev dist: " << subSols_std_dev_dist << endl;
    cout << "\n\n";


    // shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations)); // Create filtered instance (n locations, all customers)
    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations,instance->getTypeService())); // Create filtered instance (n locations, all customers)
    // cout << "\n\nFinal instance parameters:\n";
    // filtered_instance->print()
    filtered_instance->setVotedLocs(filtered_locations);

    


    atexit(printDDE);

    return filtered_instance;
}

shared_ptr<Instance> RSSV::run_CAP(uint_t thread_cnt, const string& method_sp) {



    cout << "RSSV running...\n";
    cout << "cPMP size (N): " << N << endl;
    cout << "sub-cPMP size (n): " << min(n,N) << endl;
    cout << "Subproblems cnt (M): " << M << endl;
    cout << "p: " << instance->get_p() << endl << endl;
    this->method_RSSV_sp = method_sp;
    cout << "Method to solve the Subproblems: " << method_RSSV_sp  << endl;



    if (instance->get_p() > min(n,N)){
        cout << "[ERROR] The number of facilities is smaller than the number of locations to be selected" << endl;
        cout << "[WARN]  Setting n = min(1.5 * p, N)" << endl;
        n = min(static_cast<uint_t>(1.5*instance->get_p()), N);
        // exit(1);
    } 

    sem.setCount(thread_cnt); // limit max no. of threads run in parallel
    cout << "thread cnt:  " << thread_cnt << endl;
    cout << "\n\n";

    auto start_time = tick();
    vector<thread> threads; // spawn M threads
    for (uint_t i = 1; i <= M; i += thread_cnt) {
        for (uint_t j = 0; j < thread_cnt && (i + j) <= M; ++j) {
            // cout << "Thread " << i + j << " created with seed " << i+j + seed_rssv << endl;
            threads.emplace_back(&RSSV::solveSubproblem_CAP, this, i + j + seed_rssv);
        }
        // Wait for the current batch of threads to finish before starting the next batch
        for (auto &th : threads) {
            th.join();
        }
        // Clear the threads vector for the next batch
        threads.clear();
    }


    cout << "[INFO] All subproblems solved."  << endl << endl;
    tock(start_time);

    // auto filtered_cnt = min(max(n, FILTERING_SIZE * instance->get_p()), N);
    auto filtered_cnt = n;
    auto filtered_locations = filterLocations(filtered_cnt); // Filter n locations according to voting weights
    cout << "Filtered " << filtered_cnt << " locations: ";
    for (auto fl:filtered_locations) cout << fl << " ";
    cout << endl << endl;


    auto prioritized_locations = extractPrioritizedLocations(LOC_PRIORITY_CNT);
    cout << "Extracted " << prioritized_locations.size() << " prioritized locations: ";
    for (auto pl:prioritized_locations) cout << pl << " ";
    cout << endl << endl;

    for (auto fl:filtered_locations) prioritized_locations.insert(fl);
    vector<uint_t> final_locations (prioritized_locations.begin(), prioritized_locations.end());
    

    bool extract_fixed_locations = false;
    if (extract_fixed_locations){
        cout << "Replace with fixed locations: ";
        final_locations = extractFixedLocations(final_locations);
        cout << endl << endl;
        cout << "\n\nFinal " << filtered_cnt << " locations: ";
        for (auto fl:final_locations) cout << fl << " ";
        cout << endl << endl;
        // size of final locations
        cout << "Size of final locations: " << final_locations.size() << endl << endl;
    }


    //stats
    subSols_avg_dist = subSols_avg_dist/M;
    // subSols_std_dev_dist = sqrt(subSols_std_dev_dist/M - subSols_avg_dist*subSols_avg_dist);
    subSols_std_dev_dist = subSols_std_dev_dist/M;
    cout << "\nStats: \n";
    cout << "Max dist: " << subSols_max_dist << endl;
    cout << "Min dist: " << subSols_min_dist << endl;
    cout << "avg of Avg dists: " << subSols_avg_dist << endl;
    cout << "avg Std dev dist: " << subSols_std_dev_dist << endl;
    cout << "\n\n";

    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations,instance->getTypeService())); // Create filtered instance (n locations, all customers)
    // cout << "Final instance parameters:\n";
    // filtered_instance->print();


    filtered_instance->setVotedLocs(final_locations);

    atexit(printDDE);

    return filtered_instance;
}


/*
 * Solve sub-PMP of the original problem by the TB heuristic.
 * sub-PMP considers only n locations and customers.
 */
void RSSV::solveSubproblem(int seed) {


    // // Use the seed for random number generation
    std::mt19937 gen(seed);

    sem.wait(seed);
    cout << "Solving sub-PMP " << seed << "/" << M << "..." << endl;
    auto start = tick();
    // Instance subInstance = instance->sampleSubproblem(n, n, min(instance->get_p(), MAX_SUB_P), &engine);
    Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(),seed);
    subInstance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
    if (instance->get_p() > n) {
        cout << "[ERROR] The number of facilities is greater than the number of locations to be selected" << endl;
        exit(1);
    }
    // subInstance.setCoverModel(instance->isCoverMode(),instance->getTypeSubarea());
    // subInstance.print();
    // double time_limit_subproblem = 0; // off time limit
    double time_limit_subproblem = 300; // 5 minutes

    Solution_std sol;
    if(checkClock()){
        if(method_RSSV_sp == "EXACT_PMP"){
            PMP pmp(make_shared<Instance>(subInstance), "PMP");
            pmp.setCoverModel(cover_mode,instance->getTypeSubarea());
            pmp.setCoverModel_n2(cover_mode_n2,instance->getTypeSubarea_n2());
            pmp.run();
            sol = pmp.getSolution_std();
        }else if(method_RSSV_sp == "TB_PMP"){
            TB heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            heuristic.setCoverMode_n2(cover_mode_n2);   
            // heuristic.setTimeLimit(60);
            if (time_limit_subproblem > 0) heuristic.setTimeLimit(time_limit_subproblem);
            sol = heuristic.run(false, UB_MAX_ITER);
        }else if(method_RSSV_sp == "VNS_PMP"){
            VNS heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            heuristic.setCoverMode_n2(cover_mode_n2);
            sol = heuristic.runVNS_std(false,UB_MAX_ITER);
        }else{
            cout << "Method to solve the Subproblems: " << method_RSSV_sp << " not found" << endl;
            exit(1);
        }
        if (VERBOSE) cout << "Solution_std " << seed << ": ";
        if (VERBOSE) sol.print();
        processSubsolution(make_shared<Solution_std>(sol));
        if (VERBOSE) tock(start);
        sem.notify(seed);
    }else{
        cout << "[TIMELIMIT]  Time limit exceeded to solve Sub-cPMPs " << endl;
    }


    sol.statsDistances();
    dist_t max_dist_local = sol.getMaxDist();
    dist_t min_dist_local = sol.getMinDist();
    dist_t avg_dist_local = sol.getAvgDist();
    dist_t std_dev_dist_local = sol.getStdDevDist();
    cout << "Max dist: " << max_dist_local << endl;
    cout << "Min dist: " << min_dist_local << endl;
    cout << "Avg dist: " << avg_dist_local << endl;
    cout << "Std dev dist: " << std_dev_dist_local << endl;

    mtx.lock();
        subSols_max_dist = max(subSols_max_dist,max_dist_local);
        subSols_min_dist = min(subSols_min_dist,min_dist_local);
        subSols_avg_dist += avg_dist_local;
        subSols_std_dev_dist += std_dev_dist_local;
    mtx.unlock();



    // checkClock();
}



/*
 * Solve sub-PMP of the original problem by the TB heuristic.
 * sub-PMP capacitated considers only n locations and customers.
 */
void RSSV::solveSubproblem_CAP(int seed) {
    sem.wait(seed);
    cout << "Solving sub-PMP " << seed << "/" << M << "..." << endl;
    auto start = tick();
    // Instance subInstance = instance->sampleSubproblem(n, n, min(instance->get_p(), MAX_SUB_P), &engine);
    Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(),seed);
    subInstance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
    // checkClock();
    // double time_limit_subproblem = 0; // off time limit
    double time_limit_subproblem = 300; // 5 minutes


    Solution_cap sol;
    if(checkClock()){
        if(method_RSSV_sp == "EXACT_CPMP"){
            PMP pmp(make_shared<Instance>(subInstance), "CPMP");
            pmp.setCoverModel(cover_mode,instance->getTypeSubarea());
            pmp.setCoverModel_n2(cover_mode_n2,instance->getTypeSubarea_n2());
            pmp.run();
            sol = pmp.getSolution_cap();
        }else if(method_RSSV_sp == "EXACT_CPMP_BIN"){
            PMP pmp(make_shared<Instance>(subInstance), "CPMP", true);
            pmp.setCoverModel(cover_mode,instance->getTypeSubarea());
            pmp.setCoverModel_n2(cover_mode_n2,instance->getTypeSubarea_n2());
            pmp.run();
            sol = pmp.getSolution_cap();
        }else if(method_RSSV_sp == "TB_CPMP"){
            TB heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            heuristic.setCoverMode_n2(cover_mode_n2);
            if (time_limit_subproblem > 0) heuristic.setTimeLimit(time_limit_subproblem);
            sol = heuristic.run_cap(true, UB_MAX_ITER);
        }else if(method_RSSV_sp == "VNS_CPMP"){
            VNS heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            heuristic.setCoverMode_n2(cover_mode_n2);
            sol = heuristic.runVNS_cap(method_RSSV_sp,false,UB_MAX_ITER);
        }else{
            cout << "Method to solve the Subproblems: " << method_RSSV_sp << " not found" << endl;
            exit(1);
        }

        if (VERBOSE) cout << "Solution_cap " << seed << ": ";
        if (VERBOSE) sol.print();
        processSubsolution_CAP(make_shared<Solution_cap>(sol));
        if (VERBOSE) tock(start);
        sem.notify(seed);
    }else{
        cout << "[TIMELIMIT]  Time limit exceeded to solve Sub-cPMPs " << endl;
    }

    sol.statsDistances();
    dist_t max_dist_local = sol.getMaxDist();
    dist_t min_dist_local = sol.getMinDist();
    dist_t avg_dist_local = sol.getAvgDist();
    dist_t std_dev_dist_local = sol.getStdDevDist();

    //PRINT
    cout << "Max dist: " << max_dist_local << endl;
    cout << "Min dist: " << min_dist_local << endl;
    cout << "Avg dist: " << avg_dist_local << endl;
    cout << "Std dev dist: " << std_dev_dist_local << endl;

    mtx.lock();
    subSols_max_dist = max(subSols_max_dist,max_dist_local);
    subSols_min_dist = min(subSols_min_dist,min_dist_local);
    subSols_avg_dist += avg_dist_local;
    subSols_std_dev_dist += std_dev_dist_local;
    mtx.unlock();


    // checkClock();
}

/*
 * Extract voting weights from a subproblem solution.
 * Distance to closest locations is determined from closest customer, NOT from the location in the solution (as it should be).
 */
void RSSV::processSubsolution(shared_ptr<Solution_std> solution) {
    for (auto loc_sol:solution->get_pLocations()) {
        // get closest customer in orig. instance
        auto cust_cl = instance->getClosestCust(loc_sol);
        // evaluate voting score increment for all locations in orig. instance
        weights_mutex.lock();
        for (auto loc:instance->getLocations()) {
            if (loc == loc_sol) {
                weights[loc] += 1;
            } else {
                weights[loc] += instance->getVotingScore(loc, cust_cl);
            }
        }
        weights_mutex.unlock();
    }
}


void RSSV::processSubsolution_CAP(shared_ptr<Solution_cap> solution) {
    for (auto loc_sol:solution->get_pLocations()) {
        // get closest customer in orig. instance
        auto cust_cl = instance->getClosestCust(loc_sol);
        // evaluate voting score increment for all locations in orig. instance
        weights_mutex.lock();
        for (auto loc:instance->getLocations()) {
            if (loc == loc_sol) {
                weights[loc] += 1;
            } else {
                weights[loc] += instance->getVotingScore(loc, cust_cl);
            }
        }
        weights_mutex.unlock();
    }
}

bool cmp(pair<uint_t, double>& a,
         pair<uint_t, double>& b)
{
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


    // eliminate cont last ellements in locations and add the not_in_final_locs

    // elimine last ellements in final locs wich is not in in_final_locs



    // for (int i = 0; i < cont; i++) {
    //     // check if the last element is in in_final_locs
    //     if (find(in_final_locs.begin(), in_final_locs.end(), final_locs.back()) == in_final_locs.end()) {
    //         not_in_final_locs.push_back(final_locs.back()); // add the last element in not_in_final_locs
    //         final_locs.pop_back(); // remove the last element
    //         i = i - 1; // decrement the counter
    //         if (final_locs.size() == 0) break;
    //     }else{
    //         final_locs.pop_back();
    //     }
    // }
    // for (auto loc:p_locations) {
    //     final_locs.push_back(loc);
    // }

    cout << "Replaced " << cont << " fixed locations" << endl;

    return final_locs;
    
}