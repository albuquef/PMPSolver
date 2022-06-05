#include "RSSV.hpp"
#include "utils.hpp"

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
shared_ptr<Instance> RSSV::run(int thread_cnt) {
    cout << "RSSV running...\n";
    cout << "PMP size (N): " << N << endl;
    cout << "sub-PMP size (n): " << n << endl;
    cout << "Subproblems cnt (M): " << M << endl << endl;

    sem.setCount(thread_cnt); // limit max no. of threads run in parallel

    vector<thread> threads; // spawn M threads
    for (uint_t i = 1; i <= M; i++) {
        threads.emplace_back(&RSSV::solveSubproblem, this, i);
    }

    for (auto &th:threads) { // wait for all threads
        th.join();
    }
    cout << "All subproblems solved."  << endl << endl;

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

    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(final_locations)); // Create filtered instance (n locations, all customers)
    cout << "Final instance parameters:\n";
    filtered_instance->print();

    return filtered_instance;
}

/*
 * Solve sub-PMP of the original problem by the TB heuristic.
 * sub-PMP considers only n locations and customers.
 */
void RSSV::solveSubproblem(int seed) {
    sem.wait(seed);
    cout << "Solving sub-PMP " << seed << "/" << M << "..." << endl;
    auto start = tick();
    Instance subInstance = instance->sampleSubproblem(n, n, min(instance->get_p(), MAX_SUB_P), &engine);

    checkClock();
    
    TB heuristic(make_shared<Instance>(subInstance), seed);
    auto sol = heuristic.run(false);

    cout << "Solution_std " << seed << ": ";
    sol.print();
    processSubsolution(make_shared<Solution_std>(sol));
    tock(start);
    sem.notify(seed);

    checkClock();
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
            if (DEFAULT_DISTANCE - instance->getRealDist(l, c) > TOLERANCE) cnt++;
        }
        if (cnt <= min_cnt) {
            for (auto l:instance->getLocations()) {
                if (DEFAULT_DISTANCE - instance->getRealDist(l, c) > TOLERANCE) locations.insert(l);
            }
        }
    }
    return locations;
}




