#include <thread>
#include "RSSV.hpp"
#include "TB.hpp"
#include "instance.hpp"

RSSV::RSSV(shared_ptr<Instance> instance, uint_t seed, uint_t n):instance(instance), n(n) {
    engine.seed(seed);
    N = instance->getLocations().size();
    M = 5*N/n;

    for (auto loc : instance->getLocations()) {
        weights[loc] = 0;
    }
}

/*
 * RSSV metaheuristic implementation.
 */
Solution RSSV::run(int thread_cnt) {
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

    auto filtered_locations = filterLocations(n); // Filter n locations according to voting weights
    cout << "Filtered " << n << " locations: ";
    for (auto fl:filtered_locations) cout << fl << " ";
    cout << endl << endl;

    shared_ptr<Instance> filtered_instance = make_shared<Instance>(instance->getReducedSubproblem(filtered_locations)); // Create filtered instance (n locations, all customers)
    cout << "Final instance parameters:\n";
    filtered_instance->print();

    TB heuristic(filtered_instance, 1); // solve filtered instance by the TB heuristic
    auto sol = heuristic.run(true);
    cout << "Final solution:\n";
    sol.print();
    sol.printAssignment();

    return sol;
}

/*
 * Solve sub-PMP of the original problem by the TB heuristic.
 * sub-PMP considers only n locations and customers.
 */
void RSSV::solveSubproblem(int seed) {
    sem.wait(seed);
    cout << "Solving sub-PMP " << seed << "/" << M << "..." << endl;
    auto start = tick();
    Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(), &engine);
    TB heuristic(make_shared<Instance>(subInstance), seed);
    auto sol = heuristic.run(false);

    cout << "Solution " << seed << ": ";
    sol.print();
    processSubsolution(make_shared<Solution>(sol));
    tock(start);
    sem.notify(seed);
}

/*
 * Extract voting weights from a subproblem solution.
 * Distance to closest locations is determined from closest customer, NOT from the location in the solution (as it should be).
 */
void RSSV::processSubsolution(shared_ptr<Solution> solution) {
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




