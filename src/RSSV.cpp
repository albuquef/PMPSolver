#include <thread>
#include <unistd.h>
#include "RSSV.hpp"
#include "TB.hpp"

RSSV::RSSV(Instance *instance, uint_t seed, uint_t n):instance(instance), n(n) {
    engine.seed(seed);
    N = instance->getLocations().size();
    M = 5*N/n;

    for (auto loc : instance->getLocations()) {
        weights[loc] = 0;
    }
}

Solution RSSV::run() {
    cout << "RSSV running...\n";
    cout << "PMP size (N): " << N << endl;
    cout << "sub-PMP size (n): " << n << endl;
    cout << "Subproblems cnt (M): " << M << endl;


    uint_t thread_cnt = 4;
    sem.setCount(thread_cnt);

    vector<thread> threads;
    for (uint_t i = 1; i <= M; i++) {
        threads.emplace_back(&RSSV::solveSubproblem, this, i);
    }

    for (auto &th:threads) {
        th.join();
    }

    return Solution();
}

void RSSV::solveSubproblem(uint_t seed) {
    sem.wait(seed);
    cout << "Solving sub-PMP " << seed << "/" << M << endl;
    Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(), &engine);
//    subInstance.print();
    TB heuristic(&subInstance, seed);
    auto sol = heuristic.run();

//    cout << "Solution " << seed << endl;
    processSubsolution(&sol);
    sem.notify(seed);
}

void RSSV::processSubsolution(Solution *solution) {
    for (auto loc_sol:solution->get_pLocations()) {
        // get closest customer in orig. instance
        auto cust_cl = instance->getClosestCust(loc_sol);
        // evaluate voting score increment for all locations in orig. instance
        weights_mutex.lock();
        for (auto loc:instance->getLocations()) {
            weights[loc] += instance->getVotingScore(loc, cust_cl);
        }
        weights_mutex.unlock();
    }
}




