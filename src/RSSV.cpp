#include "RSSV.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <random>

void printDDE(void){
    cout << "RSSV finished." << endl;
}

RSSV::RSSV(const shared_ptr<Instance>& instance, uint_t seed, uint_t n):instance(instance), n(n) {
    engine.seed(seed);
    N = instance->getLocations().size();
    M = LOC_FREQUENCY*N/n;

    for (auto loc : instance->getLocations()) {
        weights[loc] = DEFAULT_WEIGHT;
    }
}

/*
 * RSSV metaheuristic implementation.
 */
shared_ptr<Instance> RSSV::run(uint_t thread_cnt, string& method_sp) {
    cout << "RSSV running...\n";
    cout << "cPMP size (N): " << N << endl;
    cout << "sub-cPMP size (n): " << min(n,N) << endl;
    cout << "Subproblems cnt (M): " << M << endl << endl;
    this->method_RSSV_sp = method_sp;
    cout << "Method to solve the Subproblems: " << method_RSSV_sp  << endl;

    sem.setCount(thread_cnt); // limit max no. of threads run in parallel
    cout << "thread cnt:  " << thread_cnt << endl;
    cout << "\n\n";

    auto start_time = tick();
    vector<thread> threads; // spawn M threads
    for (uint_t i = 1; i <= M; i += thread_cnt) {
        for (uint_t j = 0; j < thread_cnt && (i + j) <= M; ++j) {
            threads.emplace_back(&RSSV::solveSubproblem, this, i + j);
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

    auto filtered_cnt = max(n, FILTERING_SIZE * instance->get_p());
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

    // shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations)); // Create filtered instance (n locations, all customers)
    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations,instance->getTypeService())); // Create filtered instance (n locations, all customers)
    // cout << "\n\nFinal instance parameters:\n";
    // filtered_instance->print();

    atexit(printDDE);

    return filtered_instance;
}

shared_ptr<Instance> RSSV::run_CAP(uint_t thread_cnt, string& method_sp) {



    cout << "RSSV running...\n";
    cout << "cPMP size (N): " << N << endl;
    cout << "sub-cPMP size (n): " << min(n,N) << endl;
    cout << "Subproblems cnt (M): " << M << endl << endl;
    this->method_RSSV_sp = method_sp;
    cout << "Method to solve the Subproblems: " << method_RSSV_sp  << endl;

    if (instance->get_p() < min(n,N)){
        cout << "The number of facilities is less than the number of locations to be selected" << endl;
        exit(1);
    } 

    sem.setCount(thread_cnt); // limit max no. of threads run in parallel
    cout << "thread cnt:  " << thread_cnt << endl;
    cout << "\n\n";

    vector<thread> threads; // spawn M threads
    // for (uint_t i = 1; i <= M; i++) {
    //     threads.emplace_back(&RSSV::solveSubproblem_CAP, this, i);
    // }
    for (uint_t i = 1; i <= M; i += thread_cnt) {
        for (uint_t j = 0; j < thread_cnt && (i + j) <= M; ++j) {
            threads.emplace_back(&RSSV::solveSubproblem_CAP, this, i + j);
        }
        // Wait for the current batch of threads to finish before starting the next batch
        for (auto &th : threads) {
            th.join();
        }
        // Clear the threads vector for the next batch
        threads.clear();
    }
    cout << "[INFO] All subproblems solved."  << endl << endl;

    auto filtered_cnt = max(n, FILTERING_SIZE * instance->get_p());
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

    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations,instance->getTypeService())); // Create filtered instance (n locations, all customers)
    // cout << "Final instance parameters:\n";
    // filtered_instance->print();

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
    // subInstance.setCoverModel(instance->isCoverMode(),instance->getTypeSubarea());
    // subInstance.print();
    // double time_limit_subproblem = 0; // off time limit
    double time_limit_subproblem = 300; // 5 minutes

    Solution_std sol;
    if(checkClock()){
        if(method_RSSV_sp == "EXACT_PMP"){
            PMP pmp(make_shared<Instance>(subInstance), "PMP");
            pmp.setCoverModel(cover_mode,instance->getTypeSubarea());
            pmp.run();
            sol = pmp.getSolution_std();
        }else if(method_RSSV_sp == "TB_PMP"){
            TB heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            // heuristic.setTimeLimit(60);
            if (time_limit_subproblem > 0) heuristic.setTimeLimit(time_limit_subproblem);
            sol = heuristic.run(false, UB_MAX_ITER);
        }else if(method_RSSV_sp == "VNS_PMP"){
            VNS heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
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
    // checkClock();
}



/*
 * Solve sub-PMP of the original problem by the TB heuristic.
 * sub-PMP considers only n locations and customers.
 */
void RSSV::solveSubproblem_CAP(int seed) {
    sem.wait(seed);
    cout << "Solving sub-PMP " << seed << "/" << M << "..." << endl;
    auto start = tick();
    // Instance subInstance = instance->sampleSubproblem(n, n, min(instance->get_p(), MAX_SUB_P), &engine);
    Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(),seed);
    // checkClock();
    // double time_limit_subproblem = 0; // off time limit
    double time_limit_subproblem = 300; // 5 minutes


    Solution_cap sol;
    if(checkClock()){
        if(method_RSSV_sp == "EXACT_CPMP"){
            PMP pmp(make_shared<Instance>(subInstance), "CPMP");
            pmp.setCoverModel(cover_mode,instance->getTypeSubarea());
            pmp.run();
            sol = pmp.getSolution_cap();
        }else if(method_RSSV_sp == "EXACT_CPMP_BIN"){
            PMP pmp(make_shared<Instance>(subInstance), "CPMP", true);
            pmp.setCoverModel(cover_mode,instance->getTypeSubarea());
            pmp.run();
            sol = pmp.getSolution_cap();
        }else if(method_RSSV_sp == "TB_CPMP"){
            TB heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
            if (time_limit_subproblem > 0) heuristic.setTimeLimit(time_limit_subproblem);
            sol = heuristic.run_cap(true, UB_MAX_ITER);
        }else if(method_RSSV_sp == "VNS_CPMP"){
            VNS heuristic(make_shared<Instance>(subInstance), seed);
            heuristic.setCoverMode(cover_mode);
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